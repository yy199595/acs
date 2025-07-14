//
// Created by 64658 on 2025/6/16.
//

#ifndef APP_CENSORFACTORY_H
#define APP_CENSORFACTORY_H


#include <string>
#include <unordered_map>
#include <queue>
#include <vector>
namespace censor
{
	extern std::u32string utf8_to_u32(const std::string& str);
	extern std::string u32_to_utf8(const std::u32string& str);

	class Factory
	{
		struct Node
		{
			std::unordered_map<char32_t, Node*> children;
			Node* fail = nullptr;
			std::vector<int> wordLens;
		};
	private:
		Node* root;

	public:
		Factory() : root(new Node) { }
		~Factory();
	public:
		void Build();
		bool Check(const std::string& text);
		void Insert(const std::string& text);
		bool LoadFromFile(const std::string & path);
		unsigned int Mask(std::string& text, char maskChar = '*');
	};
}


#endif //APP_CENSORFACTORY_H
