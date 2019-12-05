#include "tokenizer/tokenizer.h"

#include <cctype>
#include <sstream>
#include <3rd_party/fmt/include/fmt/format.h>


namespace miniplc0 {

	std::pair<std::optional<Token>, std::optional<CompilationError>> Tokenizer::NextToken() {
		if (!_initialized)
			readAll();
		if (_rdr.bad())
			return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(0, 0, ErrorCode::ErrStreamError));
		if (isEOF())
			return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(0, 0, ErrorCode::ErrEOF));
		auto p = nextToken();
		if (p.second.has_value())
			return std::make_pair(p.first, p.second);
		auto err = checkToken(p.first.value());
		if (err.has_value())
			return std::make_pair(p.first, err.value());
		return std::make_pair(p.first, std::optional<CompilationError>());
	}

	std::pair<std::vector<Token>, std::optional<CompilationError>> Tokenizer::AllTokens() {
		std::vector<Token> result;
		while (true) {
			auto p = NextToken();
			if (p.second.has_value()) {
				if (p.second.value().GetCode() == ErrorCode::ErrEOF)
					return std::make_pair(result, std::optional<CompilationError>());
				else
					return std::make_pair(std::vector<Token>(), p.second);
			}
			result.emplace_back(p.first.value());
		}
	}

	// 注意：这里的返回值中 Token 和 CompilationError 只能返回一个，不能同时返回。
	std::pair<std::optional<Token>, std::optional<CompilationError>> Tokenizer::nextToken() {
		// 用于存储已经读到的组成当前token字符
		std::stringstream ss;
		// 分析token的结果，作为此函数的返回值
		std::pair<std::optional<Token>, std::optional<CompilationError>> result;
		// <行号，列号>，表示当前token的第一个字符在源代码中的位置
		std::pair<int64_t, int64_t> pos;
		// 记录当前自动机的状态，进入此函数时是初始状态
		DFAState current_state = DFAState::INITIAL_STATE;
		// 这是一个死循环，除非主动跳出
		// 每一次执行while内的代码，都可能导致状态的变更
		while (true) {
			// 读一个字符，请注意auto推导得出的类型是std::optional<char>
			// 这里其实有两种写法
			// 1. 每次循环前立即读入一个 char
			// 2. 只有在可能会转移的状态读入一个 char
			// 因为我们实现了 unread，为了省事我们选择第一种
			auto current_char = nextChar();
			// 针对当前的状态进行不同的操作
			switch (current_state) {

				// 初始状态
				// 这个 case 我们给出了核心逻辑，但是后面的 case 不用照搬。
			case INITIAL_STATE: {
				// 已经读到了文件尾
				if (!current_char.has_value())
					// 返回一个空的token，和编译错误ErrEOF：遇到了文件尾
					return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(0, 0, ErrEOF));

				// 获取读到的字符的值，注意auto推导出的类型是char
				auto ch = current_char.value();
				// 标记是否读到了不合法的字符，初始化为否
				auto invalid = false;
                pos = previousPos();

				// 使用了自己封装的判断字符类型的函数，定义于 tokenizer/utils.hpp
				// see https://en.cppreference.com/w/cpp/string/byte/isblank
				if (isspace(ch)) // 读到的字符是空白字符（空格、换行、制表符等）
					current_state = DFAState::INITIAL_STATE; // 保留当前状态为初始状态，此处直接break也是可以的
				else if (!miniplc0::isprint(ch)) // control codes and backspace
					invalid = true;
				else if (miniplc0::isalpha(ch)) // 读到的字符是英文字母
					current_state = DFAState::IDENTIFIER_STATE; // 切换到标识符的状态
				else {

					if(ch=='+')
					    return std::make_pair(std::make_optional<Token>(TokenType::PLUS,'+',pos,currentPos()),std::optional<CompilationError>());
					else if(ch=='-')
                        return std::make_pair(std::make_optional<Token>(TokenType::MINUS,'-',pos,currentPos()),std::optional<CompilationError>());
                    else if(ch=='*')
                        return std::make_pair(std::make_optional<Token>(TokenType::MULTIPLICATION,'*',pos,currentPos()),std::optional<CompilationError>());
                    else if(ch=='/')
                        return std::make_pair(std::make_optional<Token>(TokenType::DIVISION,'/',pos,currentPos()),std::optional<CompilationError>());
                    else if(ch=='(')
                        return std::make_pair(std::make_optional<Token>(TokenType::LEFT_SMALL_BRACKET,'(',pos,currentPos()),std::optional<CompilationError>());
                    else if(ch==')')
                        return std::make_pair(std::make_optional<Token>(TokenType::RIGHT_SMALL_BRACKET,')',pos,currentPos()),std::optional<CompilationError>());
                    else if(ch=='{')
                        return std::make_pair(std::make_optional<Token>(TokenType::LEFT_BIG_BRACKET,'{',pos,currentPos()),std::optional<CompilationError>());
                    else if(ch=='}')
                        return std::make_pair(std::make_optional<Token>(TokenType::RIGHT_BIG_BRACKET,'}',pos,currentPos()),std::optional<CompilationError>());
                    else if(ch=='[')
                        return std::make_pair(std::make_optional<Token>(TokenType::LEFT_MIDDLE_BRACKET,'[',pos,currentPos()),std::optional<CompilationError>());
                    else if(ch==']')
                        return std::make_pair(std::make_optional<Token>(TokenType::RIGHT_MIDDLE_BRACKET,']',pos,currentPos()),std::optional<CompilationError>());
                    else if(ch==';')
                        return std::make_pair(std::make_optional<Token>(TokenType::SEMICOLON,';',pos,currentPos()),std::optional<CompilationError>());
                    else if(ch==',')
                        return std::make_pair(std::make_optional<Token>(TokenType::DOUHAO,',',pos,currentPos()),std::optional<CompilationError>());

                    else if(ch=='=')
                    {
                        current_char=nextChar();
                        if(current_char.value()=='=')
                            return std::make_pair(std::make_optional<Token>(TokenType::EQUAL_EQUAL,'==',pos,currentPos()),std::optional<CompilationError>());
                        else
                        {
                            unreadLast();
                            return std::make_pair(std::make_optional<Token>(TokenType::EQUAL,'=',pos,currentPos()),std::optional<CompilationError>());

                        }


                    }
                    else if(ch=='<')
                    {
                        current_char=nextChar();
                        if(current_char.value()=='=')
                            return std::make_pair(std::make_optional<Token>(TokenType::LESS_EQUAL,'<',pos,currentPos()),std::optional<CompilationError>());
                        else
                        {
                            unreadLast();
                            return std::make_pair(std::make_optional<Token>(TokenType::LESS,'<',pos,currentPos()),std::optional<CompilationError>());

                        }
                    }
                    else if(ch=='>')
                    {
                        current_char=nextChar();
                        if(current_char.value()=='=')
                            return std::make_pair(std::make_optional<Token>(TokenType::GREATER_EQUAL,'>',pos,currentPos()),std::optional<CompilationError>());
                        else
                        {
                            unreadLast();
                            return std::make_pair(std::make_optional<Token>(TokenType::GREATER,'>',pos,currentPos()),std::optional<CompilationError>());

                        }
                    }
                    else if(ch=='!')
                    {
                        current_char=nextChar();
                        if(current_char.value()=='=')
                            return std::make_pair(std::make_optional<Token>(TokenType::NO_EQUAL,'!',pos,currentPos()),std::optional<CompilationError>());
                        else
                        {
                            unreadLast();
                            return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(pos, ErrorCode::ErrInvalidInput));


                        }
                    }
                    else if(ch=='0')
                    {
                        ss << ch;
                        current_char=nextChar();

                        if(current_char.value()=='x'||current_char.value()=='X') {

                            current_state = DFAState::Hexa_STATE;
                            ss << current_char.value();

                        }
                        else {
                            unreadLast();
                            return std::make_pair(std::make_optional<Token>(TokenType::INTEGER, 0, pos, currentPos()),
                                                  std::optional<CompilationError>());

                        }
                    }
                    else if(isnozerodigit(ch))
                    {

                        current_state=Integer_STATE;

                    }



                }
				// 如果读到的字符导致了状态的转移，说明它是一个token的第一个字符
				if (current_state != DFAState::INITIAL_STATE)
					pos = previousPos(); // 记录该字符的的位置为token的开始位置
				// 读到了不合法的字符
				if (invalid) {
					// 回退这个字符
					unreadLast();
					// 返回编译错误：非法的输入
					return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(pos, ErrorCode::ErrInvalidInput));
				}
				// 如果读到的字符导致了状态的转移，说明它是一个token的第一个字符
				if (current_state != DFAState::INITIAL_STATE&&current_state!=DFAState::Hexa_STATE) // ignore white spaces
					ss << ch; // 存储读到的字符
				break;
			}
			case IDENTIFIER_STATE:
            {
                if (!current_char.has_value())
                {
                    std::string result;

                    ss >> result;


                    if(result=="const")
                        return std::make_pair(std::make_optional<Token>(TokenType::CONST, result, pos, currentPos()), std::optional<CompilationError>());
                    else if(result=="void")
                        return std::make_pair(std::make_optional<Token>(TokenType::VOID, result, pos, currentPos()), std::optional<CompilationError>());
                    else if(result=="int")
                        return std::make_pair(std::make_optional<Token>(TokenType::INT, result, pos, currentPos()), std::optional<CompilationError>());
                    else if(result=="char")
                        return std::make_pair(std::make_optional<Token>(TokenType::CHAR, result, pos, currentPos()), std::optional<CompilationError>());
                    else if(result=="double")
                        return std::make_pair(std::make_optional<Token>(TokenType::DOUBLE, result, pos, currentPos()), std::optional<CompilationError>());
                    else if(result=="struct")
                        return std::make_pair(std::make_optional<Token>(TokenType::STRUCT, result, pos, currentPos()), std::optional<CompilationError>());
                    else if(result=="if")
                        return std::make_pair(std::make_optional<Token>(TokenType::IF, result, pos, currentPos()), std::optional<CompilationError>());
                    else if(result=="else")
                        return std::make_pair(std::make_optional<Token>(TokenType::ELSE, result, pos, currentPos()), std::optional<CompilationError>());
                    else if(result=="switch")
                        return std::make_pair(std::make_optional<Token>(TokenType::SWITCH, result, pos, currentPos()), std::optional<CompilationError>());
                    else if(result=="case")
                        return std::make_pair(std::make_optional<Token>(TokenType::CASE, result, pos, currentPos()), std::optional<CompilationError>());
                    else if(result=="default")
                        return std::make_pair(std::make_optional<Token>(TokenType::DEFAULT, result, pos, currentPos()), std::optional<CompilationError>());
                    else if(result=="while")
                        return std::make_pair(std::make_optional<Token>(TokenType::WHILE, result, pos, currentPos()), std::optional<CompilationError>());
                    else if(result=="for")
                        return std::make_pair(std::make_optional<Token>(TokenType::FOR, result, pos, currentPos()), std::optional<CompilationError>());
                    else if(result=="do")
                        return std::make_pair(std::make_optional<Token>(TokenType::DO, result, pos, currentPos()), std::optional<CompilationError>());
                    else if(result=="return")
                        return std::make_pair(std::make_optional<Token>(TokenType::RETURN, result, pos, currentPos()), std::optional<CompilationError>());
                    else if(result=="break")
                        return std::make_pair(std::make_optional<Token>(TokenType::BREAK, result, pos, currentPos()), std::optional<CompilationError>());
                    else if(result=="continue")
                        return std::make_pair(std::make_optional<Token>(TokenType::CONTINUE, result, pos, currentPos()), std::optional<CompilationError>());
                    else if(result=="print")
                        return std::make_pair(std::make_optional<Token>(TokenType::PRINT, result, pos, currentPos()), std::optional<CompilationError>());
                    else if(result=="scan")
                        return std::make_pair(std::make_optional<Token>(TokenType::SCAN, result, pos, currentPos()), std::optional<CompilationError>());
                    else
                        return std::make_pair(std::make_optional<Token>(TokenType::IDENTIFIER, result, pos, currentPos()), std::optional<CompilationError>());

                }
                else if(miniplc0::isalpha(current_char.value())||miniplc0::isdigit(current_char.value()))
                {

                    ss << current_char.value();

                }
                else
                {

                    unreadLast();
                    std::string result;

                    ss >> result;

                    if(result=="const")
                        return std::make_pair(std::make_optional<Token>(TokenType::CONST, result, pos, currentPos()), std::optional<CompilationError>());
                    else if(result=="void")
                        return std::make_pair(std::make_optional<Token>(TokenType::VOID, result, pos, currentPos()), std::optional<CompilationError>());
                    else if(result=="int")
                        return std::make_pair(std::make_optional<Token>(TokenType::INT, result, pos, currentPos()), std::optional<CompilationError>());
                    else if(result=="char")
                        return std::make_pair(std::make_optional<Token>(TokenType::CHAR, result, pos, currentPos()), std::optional<CompilationError>());
                    else if(result=="double")
                        return std::make_pair(std::make_optional<Token>(TokenType::DOUBLE, result, pos, currentPos()), std::optional<CompilationError>());
                    else if(result=="struct")
                        return std::make_pair(std::make_optional<Token>(TokenType::STRUCT, result, pos, currentPos()), std::optional<CompilationError>());
                    else if(result=="if")
                        return std::make_pair(std::make_optional<Token>(TokenType::IF, result, pos, currentPos()), std::optional<CompilationError>());
                    else if(result=="else")
                        return std::make_pair(std::make_optional<Token>(TokenType::ELSE, result, pos, currentPos()), std::optional<CompilationError>());
                    else if(result=="switch")
                        return std::make_pair(std::make_optional<Token>(TokenType::SWITCH, result, pos, currentPos()), std::optional<CompilationError>());
                    else if(result=="case")
                        return std::make_pair(std::make_optional<Token>(TokenType::CASE, result, pos, currentPos()), std::optional<CompilationError>());
                    else if(result=="default")
                        return std::make_pair(std::make_optional<Token>(TokenType::DEFAULT, result, pos, currentPos()), std::optional<CompilationError>());
                    else if(result=="while")
                        return std::make_pair(std::make_optional<Token>(TokenType::WHILE, result, pos, currentPos()), std::optional<CompilationError>());
                    else if(result=="for")
                        return std::make_pair(std::make_optional<Token>(TokenType::FOR, result, pos, currentPos()), std::optional<CompilationError>());
                    else if(result=="do")
                        return std::make_pair(std::make_optional<Token>(TokenType::DO, result, pos, currentPos()), std::optional<CompilationError>());
                    else if(result=="return")
                        return std::make_pair(std::make_optional<Token>(TokenType::RETURN, result, pos, currentPos()), std::optional<CompilationError>());
                    else if(result=="break")
                        return std::make_pair(std::make_optional<Token>(TokenType::BREAK, result, pos, currentPos()), std::optional<CompilationError>());
                    else if(result=="continue")
                        return std::make_pair(std::make_optional<Token>(TokenType::CONTINUE, result, pos, currentPos()), std::optional<CompilationError>());
                    else if(result=="print")
                        return std::make_pair(std::make_optional<Token>(TokenType::PRINT, result, pos, currentPos()), std::optional<CompilationError>());
                    else if(result=="scan")
                        return std::make_pair(std::make_optional<Token>(TokenType::SCAN, result, pos, currentPos()), std::optional<CompilationError>());
                    else
                        return std::make_pair(std::make_optional<Token>(TokenType::IDENTIFIER, result, pos, currentPos()), std::optional<CompilationError>());

                }


			    break;
            }
			    case Hexa_STATE:
                {


                    if(!current_char.has_value())
                    {
                        std::string sss;
                        long long int num=0;
                        ss >> sss;
                        const char *c=sss.c_str();
                        if(c[2]=='\0')
                            return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(pos,ErrorCode::ErrIntegerOverflow));

                        int i=0;
                        for(i=2;c[i]!='\0';i++)
                        {
                            int a;
                            if(isdigit(c[i]))
                            {
                                a=c[i]-'0';
                            }
                            else
                            {
                                switch (c[i]){
                                    case 'A':
                                    {
                                        a=10;
                                        break;
                                    }
                                    case 'B':
                                    {
                                        a=11;
                                        break;
                                    }
                                    case 'C':
                                    {
                                        a=12;
                                        break;
                                    }
                                    case 'D':
                                    {
                                        a=13;
                                        break;
                                    }
                                    case 'E':
                                    {
                                        a=14;
                                        break;
                                    }
                                    case 'F':
                                    {
                                        a=15;
                                        break;
                                    }
                                    case 'a':
                                    {
                                        a=10;
                                        break;
                                    }
                                    case 'b':
                                    {
                                        a=11;
                                        break;
                                    }
                                    case 'c':
                                    {
                                        a=12;
                                        break;
                                    }
                                    case 'd':
                                    {
                                        a=13;
                                        break;
                                    }
                                    case 'e':
                                    {
                                        a=14;
                                        break;
                                    }
                                    case 'f':
                                    {
                                        a=15;
                                        break;
                                    }

                                }

                            }
                            num=num*16+a;

                            if(num>2147483647)
                                return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(pos,ErrorCode::ErrIntegerOverflow));


                        }
                        int nnn=num;
                        return std::make_pair(std::make_optional<Token>(TokenType::INTEGER, nnn, pos, currentPos()), std::optional<CompilationError>());




                    }
                    else if(ishexadecimaldigit(current_char.value())||isdigit(current_char.value()))
                    {

                        ss<<current_char.value();



                    }
                    else
                    {

                        unreadLast();
                        std::string sss;
                        long long int num=0;
                        ss >> sss;
                        const char *c=sss.c_str();
                        if(c[2]=='\0')
                            return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(pos,ErrorCode::ErrIntegerOverflow));
                        int i=0;
                        for(i=2;c[i]!='\0';i++) {
                            int a;
                            if (isdigit(c[i])) {
                                a = c[i] - '0';
                            } else {
                                switch (c[i]) {
                                    case 'A': {
                                        a = 10;
                                        break;
                                    }
                                    case 'B': {
                                        a = 11;
                                        break;
                                    }
                                    case 'C': {
                                        a = 12;
                                        break;
                                    }
                                    case 'D': {
                                        a = 13;
                                        break;
                                    }
                                    case 'E': {
                                        a = 14;
                                        break;
                                    }
                                    case 'F': {
                                        a = 15;
                                        break;
                                    }
                                    case 'a': {
                                        a = 10;
                                        break;
                                    }
                                    case 'b': {
                                        a = 11;
                                        break;
                                    }
                                    case 'c': {
                                        a = 12;
                                        break;
                                    }
                                    case 'd': {
                                        a = 13;
                                        break;
                                    }
                                    case 'e': {
                                        a = 14;
                                        break;
                                    }
                                    case 'f': {
                                        a = 15;
                                        break;
                                    }

                                }

                            }
                            num = num * 16 + a;
                            if (num > 2147483647)
                                return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(pos,
                                                                                                                   ErrorCode::ErrIntegerOverflow));
                        }
                        int nnn=num;
                        Token token=Token(TokenType::INTEGER, nnn, pos, currentPos());
                        return std::make_pair(std::make_optional<Token>(TokenType::INTEGER, nnn, pos, currentPos()), std::optional<CompilationError>());
                    }
                    break;



                }
			    case Integer_STATE:
                {
                    if(!current_char.has_value())
                    {

                        std::string sss;
                        long long int num=0;
                        ss >> sss;
                        const char *c=sss.c_str();
                        int i=0;
                        for(i=0;c[i]!='\0';i++)
                        {
                            int a;
                            a=c[i]-'0';
                            num=num*10+a;

                            if(num>2147483647)
                                return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(pos,ErrorCode::ErrIntegerOverflow));
                        }
                        int nnn=num;
                        return std::make_pair(std::make_optional<Token>(TokenType::INTEGER, nnn, pos, currentPos()), std::optional<CompilationError>());





                    }
                    else if(isdigit(current_char.value()))
                    {
                        ss << current_char.value();
                    }
                    else if(isalpha(current_char.value()))
                    {
                        ss << current_char.value();
                        current_state=IDENTIFIER_STATE;
                    }
                    else
                    {
                        unreadLast();
                        std::string sss;
                        long long int num=0;
                        ss >> sss;
                        const char *c=sss.c_str();
                        int i=0;
                        for(i=0;c[i]!='\0';i++)
                        {
                            int a;
                            a=c[i]-'0';
                            num=num*10+a;

                            if(num>2147483647)
                                return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(pos,ErrorCode::ErrIntegerOverflow));
                        }
                        int nnn=num;
                        return std::make_pair(std::make_optional<Token>(TokenType::INTEGER, nnn, pos, currentPos()), std::optional<CompilationError>());

                    }
                    break;


                }
			default: {
                DieAndPrint("unhandled state.");
                break;
            }
			}
		}
		// 预料之外的状态，如果执行到了这里，说明程序异常
		return std::make_pair(std::optional<Token>(), std::optional<CompilationError>());
	}

	std::optional<CompilationError> Tokenizer::checkToken(const Token& t) {
		switch (t.GetType()) {
			case IDENTIFIER: {
				auto val = t.GetValueString();
				if (miniplc0::isdigit(val[0]))
					return std::make_optional<CompilationError>(t.GetStartPos().first, t.GetStartPos().second, ErrorCode::ErrInvalidIdentifier);
				break;
			}
		default:
			break;
		}
		return {};
	}

	void Tokenizer::readAll() {
		if (_initialized)
			return;
		for (std::string tp; std::getline(_rdr, tp);)
			_lines_buffer.emplace_back(std::move(tp + "\n"));
		_initialized = true;
		_ptr = std::make_pair<int64_t, int64_t>(0, 0);
		return;
	}

	// Note: We allow this function to return a postion which is out of bound according to the design like std::vector::end().
	std::pair<uint64_t, uint64_t> Tokenizer::nextPos() {
		if (_ptr.first >= _lines_buffer.size())
			DieAndPrint("advance after EOF");
		if (_ptr.second == _lines_buffer[_ptr.first].size() - 1)
			return std::make_pair(_ptr.first + 1, 0);
		else
			return std::make_pair(_ptr.first, _ptr.second + 1);
	}

	std::pair<uint64_t, uint64_t> Tokenizer::currentPos() {
		return _ptr;
	}

	std::pair<uint64_t, uint64_t> Tokenizer::previousPos() {
		if (_ptr.first == 0 && _ptr.second == 0)
			DieAndPrint("previous position from beginning");
		if (_ptr.second == 0)
			return std::make_pair(_ptr.first - 1, _lines_buffer[_ptr.first - 1].size() - 1);
		else
			return std::make_pair(_ptr.first, _ptr.second - 1);
	}

	std::optional<char> Tokenizer::nextChar() {
		if (isEOF())
			return {}; // EOF
		auto result = _lines_buffer[_ptr.first][_ptr.second];
		_ptr = nextPos();
		return result;
	}

	bool Tokenizer::isEOF() {
		return _ptr.first >= _lines_buffer.size();
	}

	// Note: Is it evil to unread a buffer?
	void Tokenizer::unreadLast() {
		_ptr = previousPos();
	}

	bool Tokenizer::isnondigit(char c){
        return std::isalpha(c);

    }
    bool Tokenizer::ishexadecimaldigit(char c){
        if(c=='a'||c=='b'||c=='c'||c=='d'||c=='e'||c=='f')
            return true;
        else if(c=='A'||c=='B'||c=='C'||c=='D'||c=='E'||c=='F')
            return true;
        return false;

    }
    bool Tokenizer::isdigit(char c){
        if(c=='0'||c=='1'||c=='2'||c=='3'||c=='4'||c=='5')
            return true;
        else if(c=='6'||c=='7'||c=='8'||c=='9')
            return true;
        return false;

    }

    bool Tokenizer::isnozerodigit(char c) {
        if(c=='1'||c=='2'||c=='3'||c=='4'||c=='5')
            return true;
        else if(c=='6'||c=='7'||c=='8'||c=='9')
            return true;
        return false;


	}
    bool Tokenizer::isspace(char c) {
        if(c==32||c==9||c==10||c==13)
            return true;
        return false;


    }

}