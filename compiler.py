import sys
import mylexer
import myparser
import mytypechecker

def main():

    # flag = sys.argv[1]
    # file_spec = sys.argv[2]
    flag = '-t'
    file_spec = 'test.jpl'

    if flag[0] != '-':
        temp = flag
        flag = file_spec
        file_spec = temp
    try:
        file_reader = open(file_spec, 'r')
        file = ''.join(file_reader.readlines())
        file_reader.close()
    except Exception as error:
        print("Compilation failed! " + error.__str__())
        exit(0)

    if flag == '-l':
        try:
            newlexer = mylexer.Lexer(file)
            newlexer.runner()
            for x in newlexer.tokens:
                if x.t == 'NEWLINE':
                    print('NEWLINE')
                elif x.t == 'END_OF_FILE':
                    print('END_OF_FILE')
                else:
                    res = x.t + ' \'' + x.text + '\''
                    print(res)
            print('Compilation succeeded: lexical analysis complete')

        except mylexer.LexerError as error:
            print(error.message)
            exit(0)

    elif flag == '-p':

        try:
            newlexer = mylexer.Lexer(file)
            newlexer.runner()

            newparser = myparser.Parser(newlexer.tokens)
            newparser.parse()
            print(newparser.to_string())
            print('\nCompilation succeeded')
        except Exception as exception:
            # print('Compilation failed ' + exception.__str__())
            print('Compilation failed: ' + exception.__str__())
            exit(0)

    elif flag == '-t':
        try:
            newlexer = mylexer.Lexer(file)
            newlexer.runner()

            newparser = myparser.Parser(newlexer.tokens)
            newparser.parse()

            newtypechecker = mytypechecker.TypeChecker(newparser.program)
            newtypechecker.type_check()
            print(newtypechecker.to_string())
            print('\nCompilation succeeded')

        except Exception as exception:
            # print('Compilation failed ' + exception.__str__())
            print('Compilation failed: ' + exception.__str__())
            exit(0)
    else:
        print('A flag (-l, -p, -t) is required')


if __name__ == '__main__':
    main()


