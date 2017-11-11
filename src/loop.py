##
## @file LOOP.PY
## @brief PYAGW Command loop
## @AUTH Henk van Asselt, PA3BTL
##

# Global imports
import logging

# Local imports
import cmdloop
import frame

if __name__ == "__main__":

    class AGWPE_mainloop(cmdloop.CommandLoop):
        PS1='>'

        def __init__(self, _default_target = 'world'):
            pass

        @cmdloop.aliases('load', 'file')
        @cmdloop.shorthelp('load a trace file')
        @cmdloop.usage('load [FILENAME]')
        def loadCmd(self, flags, args):
            """
                Load a tracefile from disk
            """
            if flags or len(args) != 1:
                raise cmdloop.InvalidArguments
            else:
                lines = frame.load(args[0])
                print lines

        @cmdloop.aliases('debug')
        @cmdloop.shorthelp('set debug level')
        @cmdloop.usage('debug [ERROR | WARNING | INFO | DEBUG]')
        def debugCmd(self, flags, args):
            '''
            set the debug level to the desired level.
            ERROR only shows error message
            DEBUG is the most verbose option
            '''
            if flags or len(args) != 1:
                raise cmdloop.InvalidArguments
            elif args[0].upper() == 'ERROR':           
                logging.basicConfig( level=logging.ERROR, format='%(levelname)s %(message)s' )            
            elif args[0].upper() == 'WARNING':           
                logging.basicConfig( level=logging.WARNING, format='%(levelname)s %(message)s' )            
            elif args[0].upper() == 'INFO':           
                logging.basicConfig( level=logging.INFO, format='%(levelname)s %(message)s' )            
            elif args[0].upper() == 'DEBUG':           
                logging.basicConfig( level=logging.DEBUG, format='%(levelname)s %(message)s' )            
            else:
                raise cmdloop.InvalidArguments


        @cmdloop.aliases('quit','exit')
        def quitCmd(self, flags, args):
            '''
            Quit the environment.
            '''
            raise cmdloop.HaltLoop

        def _onLoopExit(self):
            raise cmdloop.HaltLoop

    AGWPE_mainloop().runLoop()