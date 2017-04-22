#include <Engine/TextureAtlas.h>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <regex>
#include <sstream>

using namespace std;

enum class TokenType {
	End,
	Name,
	Number,
	Equals,
	NewLine
};

struct Token {
	TokenType type;
	string name;
	uint32_t number;
};

Token parseToken(istream& in) {
	static regex name("[_]*[a-zA-Z][a-zA-Z_0-9]*");
	static regex number("0|([1-9][0-9]*)");

	Token token = { TokenType::End, "", 0 };
	int c;
	while ((c = in.get()) != EOF && (c == ' ' || c == '\t'));
	if (c == '\n') {
		token.type = TokenType::NewLine;
	} else if (c == '#') {
		while ((c = in.get()) != EOF && c != '\n');
		if (c != EOF) token.type = TokenType::NewLine;
	} else if (c == '=') {
		token.type = TokenType::Equals;
	} else if (c != EOF) {
		in.unget();
		in >> token.name;
		if (regex_match(token.name, name)) {
			token.type = TokenType::Name;
		} else if (regex_match(token.name, number)) {
			token.type = TokenType::Number;
			token.number = (uint32_t)stoi(token.name);
		} else {
			throw runtime_error("Syntax error.");
		}
	}

	return token;
}

namespace std {
	string to_string(TokenType t) {
		switch (t)
		{
		case TokenType::Name:
			return "name";
			break;
		case TokenType::Number:
			return "number";
			break;
		case TokenType::Equals:
			return "'='";
			break;
		case TokenType::NewLine:
			return "newline";
			break;
		default:
			return "unknown";
			break;
		}
	}
}

Token expectToken(istream& in, TokenType type) {
	Token t = parseToken(in);
	if (t.type != type) {
		throw runtime_error("Expected " + to_string(type) + ", got "
			+ to_string(t.type) + ".");
	}
	return t;
}

namespace Engine {
	TextureAtlas::TextureAtlas(const string& image, const string& meta) {
		texture = new Texture(image);

		ifstream metaFile(meta);
		Token token;
		while ((token = parseToken(metaFile)).type != TokenType::End) {
			while (token.type == TokenType::NewLine) {
				token = parseToken(metaFile);
			}
			if (token.type == TokenType::Name) {
				string name = token.name;
				expectToken(metaFile, TokenType::Equals);
				uint32_t nums[4];
				for (int i = 0; i < 4; i++) {
					token = expectToken(metaFile, TokenType::Number);
					nums[i] = token.number;
				}
				expectToken(metaFile, TokenType::NewLine);

				regions[name] = {
					nums[0] / (float)texture->getWidth(),
					nums[1] / (float)texture->getHeight(),
					nums[2] / (float)texture->getWidth(),
					nums[3] / (float)texture->getHeight()
				};
			} else if (!(token.type == TokenType::NewLine ||
				token.type == TokenType::End)) {
				throw runtime_error("Syntax error.");
			}
		}
	}

	TextureAtlas::~TextureAtlas() {
		delete texture;
	}
}