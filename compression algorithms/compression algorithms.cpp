#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <bitset>
#include <cstdlib>
#include <algorithm>
#include "HaffmanTree.h"

using namespace std;

// text
string text = "";

// Haffman
map<char, int> symbols;
HaffmanTree haffmanTree = HaffmanTree();
map<char, string> haffmanTable;
string haffmanText;

// LZW
map<string, int> LZWCodeTable;
map<int, string> LZWEncodeTable;
vector<int> LZWText;

void FileInput() {
	char symbol;
	string line;
	ifstream in("input.txt");
	if (in.is_open()) {
		while (getline(in, line)) {
			line += "\n";
			text += line;
			for (int i = 0; i < line.size(); i++) {
				symbol = line[i];
				if (symbols.contains(symbol)) {
					symbols[symbol]++;
				}
				else {
					symbols[symbol] = 1;
				}
			}
		}
	}
	in.close();
}

void TranslateInHaffmanText() {
	if (!text.size()) return;
	char symbol;
	for (int i = 0; i < text.size(); i++) {
		symbol = text[i];
		haffmanText += haffmanTable[symbol];
	}
}

void TranslateInLZWText() {
	if (!text.size()) return;
	for (int i = 0; i < 256; i++) {
		string s = "";
		s += char(i);
		LZWCodeTable[s] = i;
	}
	int code = 256;
	string str = "", ch = "";
	str += text[0];
	for (int i = 1; i < text.size(); i++) {
		ch += text[i];
		if (LZWCodeTable.find(str + ch) != LZWCodeTable.end()) {
			str += ch;
		}
		else {
			LZWText.push_back(LZWCodeTable[str]);
			LZWCodeTable[str + ch] = code;
			code++;
			str = ch;
		}
		ch = "";
	}
	LZWText.push_back(LZWCodeTable[str]);
	//LZWCodeTable.clear(); // см. OutputOfAlgorithms
}

void TranslateFromHaffmanText() {
	if (!haffmanText.size()) return;
	string code;
	for (int i = 0; i < haffmanText.size(); i++) {
		code += haffmanText[i];
		for (const auto& element : haffmanTable) {
			if (element.second == code) {
				text += element.first;
				code = "";
				break;
			}
		}
	}
}

void TranslateFromLZWText() {
	if (!LZWText.size()) return;
	for (int i = 0; i < 256; i++) {
		string s = "";
		s += char(i);
		LZWEncodeTable[i] = s;
	}
	int code = 256, old = LZWText[0], n;
	string str = LZWEncodeTable[old], ch = "";
	ch += str[0];
	text += str;
	for (int i = 0; i < LZWText.size() - 1; i++) {
		n = LZWText[i + 1];
		if (LZWEncodeTable.find(n) == LZWEncodeTable.end()) {
			str = LZWEncodeTable[old];
			str += ch;
		}
		else {
			str = LZWEncodeTable[n];
		}
		text += str;
		ch = "";
		ch += str[0];
		LZWEncodeTable[code] = LZWEncodeTable[old] + ch;
		code++;
		old = n;
	}
	//LZWEncodeTable.clear(); // см. OutputOfAlgorithms
}

void SerializeHaffmanTable(ofstream& out) {
	int codeLength;
	int tableSize = haffmanTable.size();
	out.write((char*)&tableSize, sizeof(int));
	for (const auto& element : haffmanTable) {
		codeLength = element.second.size();
		out << element.first;
		out.write((char*)&codeLength, sizeof(char));
		out << element.second;
	}
}

void SerializeHaffmanText(ofstream& out) {
	string t;
	int endInd, extraZerosVol = 0;
	for (int i = 0; i < haffmanText.size(); i += 8) {
		t = "";
		endInd = i + 8;
		if (endInd >= haffmanText.size()) {
			extraZerosVol = endInd - haffmanText.size();
			endInd = haffmanText.size();
		}
		for (int j = i; j < endInd; j++) {
			t += haffmanText[j];
		}
		out << (unsigned char)bitset<8> { t }.to_ulong();
	}
	out.write((char*)&extraZerosVol, sizeof(char));
}

void SerializeLZWText(ofstream& out) {
	for (int i = 0; i < LZWText.size(); i++) {
		out.write((char*)&LZWText[i], sizeof(short));
	}
}

void FileOutputHaffman() {
	ofstream out("output_haffman.txt", ios::binary | ios::out);
	if (out.is_open()) {
		SerializeHaffmanTable(out);
		SerializeHaffmanText(out);
	}
	out.close();
}

void FileOutputLZW() {
	ofstream out("output_LZW.txt", ios::binary | ios::out);
	if (out.is_open()) {
		SerializeLZWText(out);
	}
	out.close();
}

inline int SpliceBytesForSize(uint8_t const* data_ptr) {
	return
		(data_ptr[3] << 24) |
		(data_ptr[2] << 16) |
		(data_ptr[1] << 8) |
		data_ptr[0];
}

void DeserializeHaffmanTable(ifstream& in) {
	char tableSizeArr[sizeof(int)];
	in.read(tableSizeArr, sizeof(int));
	int tableSize = SpliceBytesForSize((uint8_t const*)tableSizeArr);
	int i = 0;
	char symbolArr[sizeof(char)];
	char codeLengthArr[sizeof(char)];
	for (int i = 0; i < tableSize; i++) {
		in.read(symbolArr, sizeof(char));
		char symbol = symbolArr[0];
		in.read(codeLengthArr, sizeof(char));
		int codeLength = codeLengthArr[0];
		char codeArr[64]; // FIXME: нужно динамически определять размер
		in.read(codeArr, codeLength);
		codeArr[codeLength] = '\0';
		string code = codeArr;
		haffmanTable[symbol] = code;
	}
}

void DeserializeHaffmanText(ifstream& in) {
	unsigned char byte;
	while (true) {
		char byteArr[1];
		in.read(byteArr, 1);
		byte = byteArr[0];
		auto s = in.tellg();
		if (in.peek() == EOF) {
			int extraZerosVol = byte;
			string rightLastByte = haffmanText.substr(haffmanText.size() - (8 - extraZerosVol), 8 - extraZerosVol);
			haffmanText.replace(haffmanText.size() - 8, 8, rightLastByte);
			break;
		}
		haffmanText += bitset<8> { byte }.to_string();
	}
}

inline short SpliceBytesForShort(uint8_t const* data_ptr) {
	return
		(data_ptr[1] << 8) |
		data_ptr[0];
}

void DeserializeLZWText(ifstream& in) {
	while (true) {
		char arr[sizeof(short)];
		in.read(arr, sizeof(short));
		LZWText.push_back(SpliceBytesForShort((uint8_t const*)arr));
		if (in.peek() == EOF) {
			break;
		}
	}
}

void FileInputHaffman() {
	ifstream in("output_haffman.txt", ios::binary | ios::in);
	if (in.is_open()) {
		DeserializeHaffmanTable(in);
		DeserializeHaffmanText(in);
	}
	in.close();
}

void FileInputLZW() {
	ifstream in("output_LZW.txt", ios::binary | ios::in);
	if (in.is_open()) {
		DeserializeLZWText(in);
	}
	in.close();
}

void OutputOfAlgorithms() {
	cout << "Haffman:\n\n"
		<< "code:" << endl;

	FileInput(); // чтение текстовых данных
	
	cout << "text:" << endl << text << endl; // текстовые данные
	
	haffmanTree.CreateHaffmanTree(symbols); // создание дерева
	haffmanTable = haffmanTree.ReturnHaffmanTable(); // получение таблицы символ - код
	
	int counter = 1;
	for (const auto& element : haffmanTable) {
		string symbol;
		symbol.append(1, element.first);
		if (symbol == "\n") {
			symbol = "\\n";
		}
		if (symbol == "\t") {
			symbol = "\\t";
		}
		if (symbol == "\r") {
			symbol = "\\r";
		}
		cout << counter << ". '" << symbol << "'" << "\t" << element.second << endl;
		counter++;
	}
	cout << endl; // таблица

	haffmanText.clear();
	TranslateInHaffmanText(); // кодирование (перевод) текстовых данных
	
	cout << "haffman text:" << endl << haffmanText << endl; // закодированные текстовые данные
	
	FileOutputHaffman(); // вывод в файл таблицы и закодированных текстовых данных
	
	cout << endl << "encode:" << endl;

	haffmanTable.clear();
	haffmanText.clear();
	FileInputHaffman(); // чтение таблицы и закодированных текстовых данных
	
	counter = 1;
	for (const auto& element : haffmanTable) {
		string symbol;
		symbol.append(1, element.first);
		if (symbol == "\n") {
			symbol = "\\n";
		}
		if (symbol == "\t") {
			symbol = "\\t";
		}
		if (symbol == "\r") {
			symbol = "\\r";
		}
		cout << counter << ". '" << symbol << "'" << "\t" << element.second << endl;
		counter++;
	}
	cout << endl; // таблица
	cout << "haffman text:" << endl << haffmanText << endl; // закодированные текстовые данные

	text.clear();
	TranslateFromHaffmanText(); // декодирование (перевод) закодированных текстовых данных
	
	cout << "text:" << endl << text << endl; // текстовые данные
	
	cout << "\n\n"; // разделитель

	cout << "LZW:\n\n"
		<< "code:" << endl;

	text.clear();
	FileInput(); // чтение текстовых данных

	cout << "text:" << endl << text << endl; // текстовые данные

	LZWText.clear();
	TranslateInLZWText(); // кодирование (перевод) текстовых данных

	counter = 1;
	for (const auto& element : LZWCodeTable) {
		if (find(LZWText.begin(), LZWText.end(), element.second) != LZWText.end()) {
			string symbol = element.first;
			if (symbol == "\n") {
				symbol = "\\n";
			}
			if (symbol == "\t") {
				symbol = "\\t";
			}
			if (symbol == "\r") {
				symbol = "\\r";
			}
			cout << counter << ". '" << symbol << "'" << "\t" << element.second << endl;
			counter++;
		}
	}
	cout << endl; // таблица
	LZWCodeTable.clear(); // обычно не сохраняется (должна удаляться в TranslateInLZWText)
	// удаляется здесь, так как мы хотим увидеть как она выглядит
	
	cout << "LZW text:" << endl;
	for (int i = 0; i < LZWText.size(); i++) {
		cout << LZWText[i] << " ";
	}
	cout << endl; // закодированные текстовые данные

	FileOutputLZW(); // вывод в файл закодированных текстовых данных

	cout << endl << "encode:" << endl;

	LZWText.clear();
	FileInputLZW(); // чтение закодированных текстовых данных

	cout << "LZW text:" << endl;
	for (int i = 0; i < LZWText.size(); i++) {
		cout << LZWText[i] << " ";
	}
	cout << endl; // закодированные текстовые данные

	text.clear();
	TranslateFromLZWText();

	counter = 1;
	for (const auto& element : LZWEncodeTable) {
		if (find(LZWText.begin(), LZWText.end(), element.first) != LZWText.end()) {
			string symbol = element.second;
			if (symbol == "\n") {
				symbol = "\\n";
			}
			if (symbol == "\t") {
				symbol = "\\t";
			}
			if (symbol == "\r") {
				symbol = "\\r";
			}
			cout << counter << ". " << element.first << "\t'" << symbol << "'" << endl;
			counter++;
		}
	}
	cout << endl; // таблица
	LZWEncodeTable.clear(); // обычно не сохраняется (должна удаляться в TranslateFromLZWText)
	// удаляется здесь, так как мы хотим увидеть как она выглядит

	cout << "text:" << endl << text << endl; // текстовые данные

	fstream inputFile("input.txt"),
		haffmanFile("output_haffman.txt"),
		LZWFile("output_LZW.txt");
	inputFile.seekg(0, ios::end);
	haffmanFile.seekg(0, ios::end);
	LZWFile.seekg(0, ios::end);
	float inputFileSize = inputFile.tellg(),
		haffmanFileSize = haffmanFile.tellg(),
		LZWFileSize = LZWFile.tellg(),
		haffmanCompressionRatio = inputFileSize / haffmanFileSize,
		LZWCompressionRatio = inputFileSize / LZWFileSize;
	cout << "input.txt size: " << inputFileSize << " bytes" << endl;
	// вес исходных текстовых данных
	cout << "output_haffman.txt size: " << haffmanFileSize << " bytes;"
		// вес закодированных текстовых данных (Haffman)
		<< "\tcompression ratio: " << haffmanCompressionRatio << endl;
	// коэффициент сжатия (Haffman)
	cout << "output_LZW.txt size: " << LZWFileSize << " bytes;"
		// вес закодированных текстовых данных (LZW)
		<< "\tcompression ratio: " << LZWCompressionRatio << endl;
	// коэффициент сжатия (LZW)
	inputFile.close();
	haffmanFile.close();
	LZWFile.close();
}

int main() {
	OutputOfAlgorithms();
	text.clear();
	symbols.clear();
	haffmanTree.Clear();
	haffmanTable.clear();
	haffmanText.clear();
	LZWCodeTable.clear();
	LZWEncodeTable.clear();
	LZWText.clear();
}