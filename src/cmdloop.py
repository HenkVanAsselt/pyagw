#!/usr/bin/env python2.4
# See LICENSE.txt for license.
# (C) 2005 - Google Inc.
# (C) 2005, 2006 - Crutcher Dunnavant <crutcher@gmail.com>

"""Base class for writing simple interactive command loop environments.

CommandLoop provides a base class for writing simple interactive user
environments.  It is designed around sub-classing, has a simple command
parser, and is trivial to initialize.

Here is a trivial little environment written using CommandLoop:

    import cmdloop

    class Hello(cmdloop.CommandLoop):
        PS1='hello>'

        @cmdloop.aliases('hello', 'hi', 'hola')
        @cmdloop.shorthelp('say hello')
        @cmdloop.usage('hello TARGET')
        def helloCmd(self, flags, args):
            '''
            Say hello to TARGET, which defaults to 'world'
            '''
            if flags or len(args) > 1:
                raise InvalidArguments
            if args:
                target = args[0]
            else:
                target = self.default_target
            print >> self.OUT, 'Hello %s!' % target

        @cmdloop.aliases('quit')
        def quitCmd(self, flags, args):
            '''
            Quit the environment.
            '''
            raise cmdloop.HaltLoop

    Hello().runLoop()

Here's a more complex example:

    import cmdloop

    class HelloGoodbye(cmdloop.CommandLoop):
        PS1='hello>'

        def __init__(self, default_target = 'world'):
            self.default_target = default_target
            self.target_list = []

        @cmdloop.aliases('hello', 'hi', 'hola')
        @cmdloop.shorthelp('say hello')
        @cmdloop.usage('hello [TARGET]')
        def helloCmd(self, flags, args):
            '''
            Say hello to TARGET, which defaults to 'world'
            '''
            if flags or len(args) > 1:
                raise cmdloop.InvalidArguments
            if args:
                target = args[0]
            else:
                target = self.default_target
            if target not in self.target_list:
                self.target_list.append(target)
            print >> self.OUT, 'Hello %s!' % target

        @cmdloop.aliases('goodbye')
        @cmdloop.shorthelp('say goodbye')
        @cmdloop.usage('goodbye TARGET')
        def goodbyeCmd(self, flags, args):
            '''
            Say goodbye to TARGET.
            '''
            if flags or len(args) != 1:
                raise cmdloop.InvalidArguments
            target = args[0]
            if target in self.target_list:
                print >> self.OUT, 'Goodbye %s!' % target
                self.target_list.remove(target)
            else:
                print >> self.OUT, "I haven't said hello to %s." % target

        @cmdloop.aliases('quit')
        def quitCmd(self, flags, args):
            '''
            Quit the environment.
            '''
            raise cmdloop.HaltLoop

        def _onLoopExit(self):
            if len(self.target_list):
                self.pushCommands(('quit',))
                for target in self.target_list:
                    self.pushCommands(('goodbye', target))
            else:
                raise cmdloop.HaltLoop

    HelloGoodbye().runLoop()
"""

__author__ = 'crutcher@gmail.com (Crutcher Dunnavant)'

import sys
import string

# used to display exceptions which arise during the loop.
import traceback

import textwrap

try:
    # for raw_input(), which uses readline if available.
    import readline
except:
    pass

class CommandLoopException(Exception):
    """Base class for all Exceptions defined in this module."""
    pass


class InvalidArguments(CommandLoopException):
    """Thrown by commands that want to signal that they received
    invalid arguments.
    """
    pass


class HaltLoop(CommandLoopException):
    """Thrown to indicate that the user has asked us to halt the current action.
    """
    pass


class ParseIncomplete(CommandLoopException):
    """Thrown by ParseCommandLine to indicate that it needs more characters in
    order to complete parsing the current command line.
    """
    pass


class CommandSyntaxError(CommandLoopException):
    """Thrown by ParseCommandLine to indicate that the current command line has
    unrecoverable syntax errors, and needs to be discarded.
    """
    pass


def parseCommandLine(cmdline):
    """Given a string, attempt to parse it in a pseudo-sh like manner.

    Args:
        cmdline - the string to parse.

    Returns:
        A list of commands.

    Ex:
        parseCommandLine('foo bar ; baz') == [['foo', 'bar'], ['baz']]
        
    Throws:
        ParseIncomplete - some escape or quoting context has not been closed,
            and the parse cannot be completed without more input.
        CommandSyntaxError - the string has unrecoverable syntax errors.

    GRAMMAR:
        # . is any character
        # $ is a newline
        # \\ is one slash, not two

        CMDS := CMD ( ';' CMD )*
        CMD := (TOKEN ('\\'$)?)*
        TOKEN := ( WORDCHAR | ESCAPEDCHAR | STRING )+
        ESCAPEDCHAR := '\\'.
        WORDCHAR := [^{string.whitespace}\\;"']]
        STRING := '"' ( [^"] | '\\"' ) * '"' | "'" ( [^'] | "\\'" ) * "'"
    """
    cur_cmd = []
    cmds = [ cur_cmd ]
    
    WHITESPACE = 0
    WORD = 1
    QUOTE = 2
    
    state = WHITESPACE
    quotechar = None
    escaped = False
    
    def descape(c):
        if c == 't':
            return '\t'
        if c == 'n':
            return '\n'
        else:
            return c
    
    for c in cmdline:
        if not escaped and c == '\\':
            escaped = True
            continue
    
        if state == WHITESPACE:
            if escaped:
                escaped = False
                if c != '\n':
                    cur_cmd.append(descape(c))
                    state = WORD
            elif c in string.whitespace:
                continue
            elif c in '\'\"':
                cur_cmd.append('')
                quotechar = c
                state = QUOTE
            elif c == ';':
                cur_cmd = []
                cmds.append(cur_cmd)
            else:
                cur_cmd.append(c)
                state = WORD
        elif state == WORD:
            if escaped:
                escaped = False
                if c != '\n':
                    cur_cmd[-1] += descape(c)
            elif c in string.whitespace:
                state = WHITESPACE
            elif c in '\'\"':
                quotechar = c
                state = QUOTE
            elif c == ';':
                cur_cmd = []
                cmds.append(cur_cmd)
                state = WHITESPACE
            else:
                cur_cmd[-1] += c
        elif state == QUOTE:
            if escaped:
                escaped = False
                cur_cmd[-1] += descape(c)
            elif c == quotechar:
                state = WORD
            else:
                cur_cmd[-1] += c
    else:
        if escaped or state == QUOTE:
            raise ParseIncomplete

    return cmds


def aliases(*strs):
    """Decorator for attaching aliases to a command."""
    def decorator(func):
        func.aliases = strs
        return func
    return decorator


def shorthelp(shorthelp_str):
    """Decorator for attaching short help to a command."""
    def decorator(func):
        func.shorthelp = shorthelp_str
        return func
    return decorator


def usage(usage_str):
    """Decorator for attaching usage to a command."""
    def decorator(func):
        func.usage = usage_str
        return func
    return decorator

def addspec(aliases=None, shorthelp=None, usage=None):
    """Decorator for attaching specs to a command.

    Supported specs:
      aliases - a set of aliases for the command.
      shorthelp - a string giving a one line help description.
      usage - a string describing command usage.
    """
    def decorator(func):
        if aliases:
            func.aliases = aliases
        if shorthelp:
            func.shorthelp = shorthelp
        if usage:
            func.usage = usage
        return func
    return decorator

class CommandLoop(object):
    """CommandLoop provides a straight forward base class for implementing
    simple command loop environments. CommandLoop is meant to be sub-classed,
    and collects its commands through introspection on the class. It does all
    of this without defining an __init__ method, so subclasses are free to
    define their own __init__ methods.

    Subclasses define new commands by defining methods whose names end in
    'Cmd', take two arguments, a dict of flag values, and a list of
    positional arguments.
    
    The command aliases are defined by the cmdloop.aliases decorator;
    the optional shorthelp is defined by the cmdloop.shorthelp decorator;
    the optional usage is decorator by the cmdloop.usage decorator; and
    the command's __doc__ string is used to show help for the command.

    To create a command with aliases 'foo', 'bar', and 'f', one would
    probably use:

    @cmdloop.aliases('foo', 'bar', 'f')
    @cmdloop.shorthelp('A line of short help')
    @cmdloop.usage('foo [ARGS]')
    def fooCmd(self, flags, args):
        '''
        Extended help, only displayed when help is requested on this command.
        '''
        # check flags and args, throw InvalidArguments if you don't like em.
        # do stuff with flags and args

    You probably want to exit the CommandLoop at some point, and while the
    user can signal this with a ^D to the prompt, a 'done', 'quit', or 'exit'
    command might be more useful. You may want to use something like the
    quitCmd below.

    def quitCmd(self, flags, args):
        '''quit exit
        Quit.
        '''
        raise cmdloop.HaltLoop

    Perhaps you only wish to exit a sub environment, and don't want to
    present a 'quit' option to your user. In this case, this doneCmd may be
    more in line with your needs.

    def doneCmd(self, flags, args):
        '''done
        Exit this sub-environment.
        '''
        raise cmdloop.HaltLoop

    CommandLoop provides a few public methods:
        pushCommandLine(cmds) - push a command line onto the command stack.
        pushCommands(cmds) - push one or more commands onto the command stack.
        runLoop() - Run the loop until HaltLoop is raised by the prompt or a
                                command.

    CommandLoop also provides a 'help' command:
        helpCmd() - command which provides CommandLoop's help functionality.

    And some help for the use of Cmds:
        cmdError() - print an error message.
        InvalidArguments - an exception to throw if your arguments are bad.
        HaltLoop - an exception to throw if you want to halt the loop.

    More ambitious users may wish to override:
        _getInput() - run to get a string from the user.
        _PS1Func() - run to get the string to use as PS1.
        _preCommand() - rewriter run on a command before it is run.
        _postCommand() - run after each command, can exit loop.
        _unknownCommand() - run when a command isn't known.
        _onLoopStart() - run before the loop starts,
                                         with the option of stopping it.
        _onLoopExit() - run when the loop is exiting,
                                        with the option of continuing.

    There are also a number of attributes of interest:
        PREAMBLE - the preamble string printed by runLoop().
        CASE_SENSITIVE - Is the shell case sensitive? Defaults to True.
	PS1 - the string which the base version of _PS1Func() returns.
	PS2 - the string which the shell uses for extended prompting.

    If you wish to use CommandLoop to build dialogs that are not
    stdin/stdout, you will need to use OUT and _getInput().
        OUT - the file descriptor used for output by all CommandLoop code,
	      You should always use print >> self.OUT in your command code.
	_getInput() - the function which gets input from the user, throws
	      HaltLoop when the user wants to exit.

    Note: runLoop() exits when HaltLoop is raised, but the CommandLoop is
        still in a valid state. If it is meaningful for your subclass, you
        can re-invoke runLoop() again at a later time, and it will resume
        processing with the next command on the command stack.
    """

    """Preamble string which is printed before the loop starts."""
    PREAMBLE = ""

    """Sets if the shell commands are case sensitive."""
    CASE_SENSITIVE = True

    """User prompt 1, used as the main prompt."""
    PS1 = '>>'
    """User prompt 2, used for multi-line input."""
    PS2 = '>'

    """The file object to use for output, defaults to sys.stdout."""
    OUT = sys.stdout

    def _getInput(self, prompt):
        """A small wrapper which adapts the exceptions thrown by raw_input() 
        to the form needed by CommandLoop. Subclasses which wish to provide
        their own input must override _getInput.

        Returns:
            a line of input

        Throws:
            HaltLoop
        """
        try:
            return raw_input(prompt)
        except (EOFError, KeyboardInterrupt):
            raise HaltLoop


    def _PS1Func(self):
        """Function called whenever PS1 is needed. By default, returns
        self.PS1, but subclasses could easily use it to display dynamic status
        information, such as the current directory, or the time.

        Returns:
            a string to be used as PS1
        """
        return self.PS1

    def _mapCommands(self):
        """Determine the commands, command aliases, and command help for this
        class through introspection on the object. Needs to be done once in the
        lifetime of an instance.
        """
        self._cmds = [ cmd
                       for cmd in [ getattr(self, name)
                                    for name in dir(self)
                                    if name.endswith('Cmd') ]
                       if hasattr(cmd, 'aliases') ]
        # Commands are listed alphabetically by their primary alias.
        # This simplifies the help loop later.
        self._cmds.sort(key=lambda a: a.aliases[0])

        self._alias2cmd = {}
        for cmd in self._cmds:
            for alias in cmd.aliases:
                if not self.CASE_SENSITIVE:
                    alias = alias.lower()
                if alias in self._alias2cmd:
                    raise CommandLoopException, \
                        'Command alias "%s" used by %s already in use by %s' % \
                            (alias,
                             cmd.__name__,
                             self._alias2cmd[alias].__name__)
                else:
                    self._alias2cmd[alias] = cmd

    @aliases('help', '?')
    @shorthelp('display help')
    @usage('help [COMMAND]')
    def helpCmd(self, flags, args):
        """Run 'help' to see a list of commands, run 'help COMMAND' to see
        extended help on a given command.
        """

        def aliasdesc(cmd):
            resp = cmd.aliases[0]
            if len(cmd.aliases) > 1:
                resp += ' (' + ', '.join(cmd.aliases[1:]) + ')'
            if hasattr(cmd, 'shorthelp'):
                resp += ' - ' + cmd.shorthelp
            return resp

        if args:
            alias = args[0]
            if not self.CASE_SENSITIVE:
                alias = alias.lower()
            if alias in self._alias2cmd:
                cmd = self._alias2cmd[alias]
                print >> self.OUT, 'Help : ' + aliasdesc(cmd) 
                if hasattr(cmd,'usage'):
                    print >> self.OUT, 'usage:', cmd.usage
                if cmd.__doc__:
                    for line in textwrap.dedent(cmd.__doc__).splitlines():
                        print >> self.OUT, '  ' + line
                elif hasattr(cmd, 'shorthelp'):
                    print >> self.OUT, '  ' + cmd.shorthelp
                return
            else:
                print >> self.OUT, "Error: no such term \"%s\"" % alias

        print >> self.OUT, 'Help : Available Commands:'
        for cmd in self._cmds:
            print >> self.OUT, '  ' + aliasdesc(cmd)

    def cmdError(self, cmd, msg):
        """
        Complain about an error with a command.

        Args:
            cmd - A bound command.
            msg - the error message.
        """
        print >> self.OUT, 'Error: %s: %s' % (cmd.aliases[0], msg)

    def _preCommand(self, cmd):
        """This method is given each command as an argument list before it is
        run.  _preCommand() may then return a command argument list, which gets
        run immediately, or None, to indicate that this command has been
        swallowed. _preCommand() may take other actions, including calling
        pushCommands().

        Classes overriding this method could use it to implement variables, or
        more complex macros.

        Returns:
            a cmd argument list, or None.
        """
        return cmd

    def _postCommand(self):
        """This method is invoked after each command is run. 

        Classes overriding this method could use it to implement variables, or
        more complex macros.

        Throws:
            HaltLoop - if we want the loop to halt.
        """
        pass

    def runLoop(self, preamble = True, help = True):
        """Run the command loop.

        Args:
            preamble=True: display self.PREAMBLE before the loop starts.
            help=True: pushCommands(('help',)) before the loop starts.
        """
        if not hasattr(self, '_cmds'):
            self._mapCommands()
        try:
            self._onLoopStart()
        except HaltLoop:
            return
        if preamble:
            print >> self.OUT, textwrap.dedent(self.PREAMBLE)
        if help:
            self.pushCommands(('help',))
        while 1:
            try:
                cmd = self._preCommand(self._nextCommand())
                if not cmd:
                    continue
                cmdname, flags, args = self._parseCommandArgs(cmd)
            except HaltLoop:
                # Print a newline, since the prompt string
                # is still on the current line.
                print
                try:
                    self._onLoopExit()
                    continue
                except HaltLoop:
                    return

            try:
                if cmdname in self._alias2cmd:
                    self._alias2cmd[cmdname](flags, args)
                    self._postCommand()
                    continue
                else:
                    self._unknownCommand(cmdname, flags, args)
                    continue
            except InvalidArguments, e:
                if cmdname in self._alias2cmd:
                    cmdname = self._alias2cmd[cmdname].aliases[0]
                # You can't use self.Error for this,
                # there may not be a cmdfunc available.
                print >> self.OUT, 'Error: %s: invalid arguments' % cmdname
                if cmdname in self._alias2cmd:
                    self.helpCmd({}, (cmdname,))
                continue
            except HaltLoop:
                try:
                    self._onLoopExit()
                    continue
                except HaltLoop:
                    return
            except Exception, e:
                traceback.print_exc()
                continue

    def _unknownCommand(self, cmdname, _flags, _args):
        """Override-able method which gets called when a command is not
        recognized.

        Args:
            cmdname - the name of the command.
            flags - the command flags.
            args - the command args.

        Throws:
            HaltLoop if the loop should exit.
        """
        print >> self.OUT, "Error: %s: command not found" % cmdname

    def _onLoopStart(self):
        """Override-able method which gets called when the loop is started.

        Throws:
            HaltLoop if the loop should exit immediately.
        """
        pass

    def _onLoopExit(self):
        """Override-able method which gets called when the loop is exiting.

        Throws:
            HaltLoop if the loop should exit.
        """
        raise HaltLoop

    def pushCommandLine(self, cmdline):
        """Push a command line onto the command stack. Multiple commands can be
        separated by a semicolon (';') character.

        Ex:
            cloop.pushCommandLine('help bar')
            cloop.pushCommandLine('help; help foo')

        The first example would push "help bar" command onto the stack, while
        the second would push a "help" command followed by a "help foo" command.
        Because this is a command stack, the execution order would be:
            help
            help foo
            help bar

        Throws:
            ParseIncomplete - if the line won't parse.
        """
        self.pushCommands(*self._parseCommandLine(cmdline))

    _cmd_stack = None
    def pushCommands(self, *cmds):
        """Push one or more commands onto the command stack. These commands must
        each be either a list or tuple of string tokens.

        Ex:
            cloop.pushCommands(('help','bar'))
            cloop.pushCommands(('help',), ('help','foo'))

        The first example would push "help bar" command onto the stack, while
        the second would push a "help" command followed by a "help foo" command.
        Because this is a command stack, the execution order would be:
            help
            help foo
            help bar
        """
        if self._cmd_stack is None:
            self._cmd_stack = []
        self._cmd_stack = list(cmds) + self._cmd_stack

    def _nextCommand(self):
        """Seek and return the next command. If the command stack is empty,
        pull more commands using self._promptUser(), then check again.

        Returns:
            A parsed command tuple (cmdname, flags, args)

        Throws:
            HaltLoop - via self._promptUser(),
                       which indicates the loop should end.
        """
        while 1:
            if not self._cmd_stack:
                self.pushCommands(*self._promptUser())

            cmd = self._cmd_stack.pop(0)
            if not cmd:
                continue

            return cmd

    def _promptUser(self):
        """Prompt the user for more input, until we receive a complete parse,
        or the user tells us to stop. This uses PS1 and PS2 for prompts.

        Returns:
            a list of commands.

        Throws:
            HaltLoop - to indicate that the user wishes to halt the loop
                without issuing another command.
        """
        user_input = ""
        while 1:
            try:
                if not user_input:
                    self.PS1 = self._PS1Func()
                    user_input = self._getInput(self.PS1 + ' ')
                else:
                    user_input += '\n' + self._getInput(self.PS2 + ' ')

                if not user_input:
                    continue

                return self._parseCommandLine(user_input)

            except ParseIncomplete:
                pass

            except CommandSyntaxError:
                user_input = ""

            except HaltLoop:
                if user_input:
                    user_input = ""
                else:
                    raise

    def _parseCommandArgs(self, args):
        """Given an argument list for a command, parse it into a command name,
        a dictionary of flag values, and a set of positional arguments (not
        including the command name). Flags and positional arguments may be
        intermixed, but the special argument '--' will force all further
        arguments to be treated positionally.

        Flags and their values:
            -x        { 'x': 1 }
            -X        { 'x': 0 }
            -xYX      { 'x': 0, 'y': 0 }
            --foo     { 'foo': 1 }
            --no-foo  { 'foo': 0 }
            --foo=7   { 'foo': '7' }

        Returns:
            (cmdname, flags, args) - a tuple of string, dict, list
        """
        cmdname = args[0]
        cmdflags = {}
        cmdargs = []
        
        for i in range(1, len(args)):
            arg = args[i]

            if arg == '--':
                cmdargs.extend(args[i+1:])
                break
            elif arg.startswith('--'):
                key = arg[2:]
                if '=' in key:
                    key, value = key.split('=', 1)
                elif key.startswith('no-'):
                    key = key[3:]
                    value = 0
                else:
                    value = 1
                cmdflags[key] = value
            elif arg.startswith('-'):
                for c in arg[1:]:
                    c_l = c.lower()
                    cmdflags[c_l] = (c == c_l)
            else:
                cmdargs.append(arg)

        if not self.CASE_SENSITIVE:
            cmdname = cmdname.lower()

        return cmdname, cmdflags, cmdargs

    def _unparseCommandArgs(self, cmdname, flags, args):
        """Given a command name, a dictionary of flag values, and a positional
        argument list, construct a command argument list.

        Args:
            cmdname - the name of the command.
            flags - a dict of the command's flags.
            args - a list of the command's positional arguments.

        Returns:
            a command argument list.
        """
        args = list(args)
        args.insert(0, '--')
        shortflags = []
        longflags = []
        for flag in flags:
            value = flags[flag]
            if len(flag) == 1 and value is None:
                shortflags.append(flag.upper())
            elif len(flag) == 1 and value == 1:
                shortflags.append(flag)
            else:
                longflags.append(flag)
        for flag in longflags:
            value = flags[flag]
            if not value:
                args.insert(0, '--no-' + flag)
            elif value == 1:
                args.insert(0, '--' + flag)
            else:
                args.insert(0, '--' + flag + '=' + value)
        args.insert(0, '-' + ''.join(shortflags))
        args.insert(0, cmdname)
        return args

    def _parseCommandLine(self, cmdline):
        """Given a string, attempt to parse it in a pseudo-sh like manner.
        This function can be overridden by those wishing to use a different
        parser.

        Returns:
            A list of commands.

        Ex:
            parseCommandLine('foo bar ; baz') == [['foo', 'bar'], ['baz']]
            
        Throws:
            ParseIncomplete - some escape or quoting context has not been
                closed, and the parse cannot be completed without more input.
            CommandSyntaxError - the string has unrecoverable syntax errors.
        """
        return parseCommandLine(cmdline)


if __name__ == "__main__":
    import time

    class Example(CommandLoop):
        """This is a sample subclass of CommandLoop,
        showing off a few things you can do with it.
        """
        
        PREAMBLE = """
        An example environment to show off how to subclass CommandLoop.
        
        Commands to try:
            args -xY --foo=7 --bar monkey
            args -xYX --foo=7 --bar monkey
            alias bar args -xYX --foo=7 --bar monkey
            alias
            bar
            bar -xC --bar=8 banana
            alias bar
            alias
        """
        
        def __init__(self, greeting='hello'):
            self.greeting = greeting
            self.aliasdict = {}
        
        def _PS1Func(self):
            return '%s world : %s>' % (self.greeting, time.asctime())
        
        def _preCommand(self, cmd):
            """This is very silly, but it shows what can be done with
            _preCommand().  Most CommandLoop subclasses won't need a
            _preCommand() at all, but it is available if they do.
            """
            if cmd[0] == 'alias':
                # This is a direct alias command call.

                if len(cmd) == 1:
                    # This is just a request for the available aliases.
                    for name in sorted(self.aliasdict):
                        print >> self.OUT, 'alias %s := %s' % \
                            (name, ' '.join(self.aliasdict[name]))
                
                elif len(cmd) == 2:
                    # This is a request to remove an alias.
                    name = cmd[1]
                    if name in self.aliasdict:
                        del self.aliasdict[name]
                
                else:
                    # This is a request to add an alias.
                    self.aliasdict[cmd[1]] = cmd[2:]
        
            elif cmd[0] in self.aliasdict:
                # This command invokes an alias.

                cname, cflags, cargs = self._parseCommandArgs(cmd)
                aname, aflags, aargs = \
                    self._parseCommandArgs(self.aliasdict[cname])
        
                # Push the expanded alias onto the stack, rather than returning
                # it. If we don't, aliases won't be able to refer to aliases.
                self.pushCommands \
                  (self._unparseCommandArgs \
                     (aname,
                      dict(aflags.items() + cflags.items()),
                      aargs + cargs))
                return None
        
            else:
                return cmd
        
        @aliases('alias')
        @shorthelp('set, list, or clear command aliases')
        def aliasCmd(self, _flags, _args):
            # This command is only here to provide help strings,
            # _preCommand() intercepts the alias command.
            pass
        
        @aliases('quit', 'exit')
        def quitCmd(self, flags, args):
            """Quit."""
            raise HaltLoop
        
        @aliases('showargs', 'args')
        @shorthelp('show command args')
        @usage('showargs [FLAGS] [PARAMS]')
        def showArgsCmd(self, flags, args):
            "Displays calling flags and arguments."
            print >> self.OUT, 'flags:', flags, 'args:', args
        
        @aliases('bad', 'b')
        @shorthelp('A \'bad\' command, which throws an unhandled error.')
        def badCmd(self, _flags, _args):
            {}['foo']
        
        @aliases('subshell')
        @shorthelp('Invoke a subshell')
        @usage('subshell [PROMPT]')
        def subshellCmd(self, flags, args):  
            """
            Invoke a subshell with the specified prompt.
            """
            if not args:
                subshell = Example()
            elif len(args) == 1:
                subshell = Example(args[0])
            else:
                raise InvalidArguments
            
            subshell.runLoop()

    Example('greetings').runLoop()


    class Hello(CommandLoop):
        CASE_SENSITIVE = False
        PS1='hello>'

        def __init__(self, default_target = 'world'):
            self.default_target = default_target
            self.target_list = []

        @aliases('hello', 'hi', 'hola')
        @shorthelp('say hello')
        @usage('hello [TARGET]')
        def helloCmd(self, flags, args):
            """Say hello to TARGET, which defaults to 'world'"""
            if flags or len(args) > 1:
                raise InvalidArguments
            if args:
                target = args[0]
            else:
                target = self.default_target
            if target not in self.target_list:
                self.target_list.append(target)
            print >> self.OUT, 'Hello %s!' % target

        @aliases('goodbye')
        @shorthelp('say goodbye')
        @usage('goodbye TARGET')
        def goodbyeCmd(self, flags, args):
            """Say goodbye to TARGET."""
            if flags or len(args) != 1:
                raise InvalidArguments
            target = args[0]
            if target in self.target_list:
                print >> self.OUT, 'Goodbye %s!' % target
                self.target_list.remove(target)
            else:
                print >> self.OUT, "I haven't said hello to %s." % target

        @aliases('quit')
        @shorthelp('quit Hello environment.')
        def quitCmd(self, flags, args):
            raise HaltLoop

        def _onLoopExit(self):
            if self.target_list:
                self.pushCommands(('quit',))
                for target in self.target_list:
                    self.pushCommands(('goodbye', target))
                return
            else:
                raise HaltLoop

    Hello().runLoop()
