#include <cstring>
#include <bitset>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <sstream>
#include <string>
#include <vector> 

/*
  This code is derived from LZW@RosettaCode for UA CS435 
*/ 
 
// Compress a string to a list of output symbols.
// The result will be written to the output iterator
// starting at "result"; the final iterator is returned.

template <typename Iterator>
Iterator compress(const std::string &uncompressed, Iterator result) {
	int dictSize = 256;						  // Dictionary starts at end of ASCII table 
	int tableSize = 65536;					  // Total size of table
	std::map<std::string,int> dictionary;	  // Dictionary map uses <string, int>
	std::string w;

	// Fill dictionary with int values
	for (int i = 0; i < dictSize; i++)
		dictionary[std::string(1, i)] = i;

	// Iterate through string
	for (std::string::const_iterator it = uncompressed.begin(); it != uncompressed.end(); ++it) {
		char c = *it;						  // c gets current char pointed to by iterator
		std::string wc = w + c;				  // wc gets previous and current char
		
		// If dictionary contains any instances of wc string
		if (dictionary.count(wc))
			w = wc;							  // w gets previous and current char (wc)
		else {
			*result++ = dictionary[w];		  // result gets value that corresponds to key w
			
			// Add wc to the dictionary.
			if (dictionary.size() < tableSize)
				dictionary[wc] = dictSize++;  // Add new value to dictionary at wc key location
			w = std::string(1, c);			  // w gets char that it is pointing to
		}
	}
 
	// If w is not empty string
	if (!w.empty())
		*result++ = dictionary[w];			  // Result gets last value associated with key w
	return result;
}
 
// Decompress a list of output ks to a string.
// "begin" and "end" must form a valid range of ints

template <typename Iterator>
std::string decompress(Iterator begin, Iterator end) {
	// Build the dictionary.
	int dictSize = 256;
	int tableSize = 65536;
	std::map<int,std::string> dictionary;
	
	// Fill dictionary with string values
	for (int i = 0; i < dictSize; i++)
		dictionary[i] = std::string(1, i);
 
	std::string w(1, *begin++);					// w gets first char of vector
	std::string result = w;						// result gets w 
	std::string entry;
	
	// Loop through entire vector
	for ( ; begin != end; begin++) {
		int k = *begin;							// k gets value pointed to by begin (starts at begin+1)
		
		// If dictionary contains any values k
		if (dictionary.count(k))
			entry = dictionary[k];				// Entry gets k value
		// If k exceeds ACII table size
		else if (k == dictSize)
			entry = w + w[0];
		else throw "Bad compressed k";

		result += entry;						// Add entry to result

		// Add w+entry[0] to the dictionary.
		if (dictionary.size() < tableSize)
			dictionary[dictSize++] = w + entry[0];	// Previous value + 1st char of current value 
		w = entry;
	}
	return result;
}

// Convert integers to string of binary characters
std::string int2BinaryString(int c, int cl) {
	std::string p = ""; //a binary code string with code length = cl
    int code = c;
	
	while (c > 0) {
		if (c % 2 == 0)
			p = "0" + p;
        else
            p = "1" + p;
        c = c >> 1;   
    }
    
	int zeros = cl - p.size();
    
	if (zeros < 0) {
		std::cout << "\nWarning: Overflow. code " << code 
				  <<" is too big to be coded by " << cl <<" bits!\n";
        p = p.substr(p.size() - cl);
    }
    else {
		for (int i = 0; i < zeros; i++)  //pad 0s to left of the binary code if needed
			p = "0" + p;
    }
    
	return p;
}

// Convert string of binary characters to int
int binaryString2Int(std::string p) {
	int code = 0;

	if (p.size() > 0) {
		if (p.at(0) == '1') 
			code = 1;
		p = p.substr(1);
    
		while (p.size() > 0) { 
			code = code << 1; 
			if (p.at(0) == '1')
				code++;
			p = p.substr(1);
		}
	}

   return code;
}

// Construct integer string made up of every byte of the
// passed in binary string
std::string binary2IntegerString(std::string binStr) {
	int remainder = binStr.length() % 8;
	binStr.append(8 - remainder, '0');
	std::string str;

	for (int i = 0; i < binStr.size(); i++) {
		str = binStr.substr(i,8);
		binStr.replace(i,8,1,(char)binaryString2Int(str));
	}
	return binStr;
}

// Construct binary string from every byte of input string
std::string string2BinaryString(std::string input) {
	std::string str;
				
	for (int i = 0; i < input.length(); i++) {
		std::bitset<8> bits((int)(input[i]));
		str.append(bits.to_string());
	}
					
	return str;
}

// Allows variable bit length values when compressing
std::string variableCompress(std::vector<int> v) {
	int bits = 9;
	int size = 256;
	std::string temp;
	std::string str;
				
	// Need to iterate through the vector to track bit lengths
	for (std::vector<int>::iterator iter = v.begin(); 
		iter != v.end(); iter++)
	{	
		// Convert a "bits" amount of data to binary string
		temp = int2BinaryString(*iter, bits);
		size++;			  // Increment size

		// Determine what size to expand to
		switch(size) {
			case 512:
				bits = 10;	
				break;
			case 1024:
				bits = 11;
				break;
			case 2048:
				bits = 12;
				break;
			case 4096:
				bits = 13;
				break;
			case 8192:
				bits = 14;
				break;
			case 16384:
				bits = 15;
				break;
			case 32768:
				bits = 16;
				break;
			default: break;
		}
		str += temp;	  // Add temp to return string
	}	
	return str;
}

// Allows expansion of variable bit-length strings
void variableExpand(std::vector<int>& v, std::string data) {
	int size = 256;		// ASCII table ends at value 255
	int bits = 9;		// Start with 9 bits
	std::string temp;	// Temp string to hold bits for each iteration of for loop

	// Convert input data to binary string in 8-bit blocks
	std::string binStr = string2BinaryString(data);

	// Loop through binStr, assigning bit patters to temp
	for (int i = 0; i < binStr.length(); i++) {
		temp += binStr[i];
		
		// When length of temp exceeds the number of bits
		if (temp.length() >= bits) {
			v.push_back(binaryString2Int(temp));	// Convert to int and append to vector
			size++;									// Increment size
			
			// Determine what size to expand to
			switch(size) {
				case 512:
					bits = 10;	
					break;
				case 1024:
					bits = 11;
					break;
				case 2048:
					bits = 12;
					break;
				case 4096:
					bits = 13;
					break;
				case 8192:
					bits = 14;
					break;
				case 16384:
					bits = 15;
					break;
				case 32768:
					bits = 16;
					break;
				default: break;
			}
			temp.clear();		// Clear temp string for next iteration of loop
		}
	}
}

int main(int argc, char** argv) {
	try {
		// Verify that that correct number of arguments have been passed in
		if (argc < 3)
			throw "USAGE: ./lzw435 OPERATION (c/e) FILENAME";

		std::vector<int> compressed;			  // Integer vector to hold compressed values
		const char op = tolower(*argv[1]);		  // Hold operation argument

		// Open input file	
		std::string filename = argv[2];
		std::ifstream inputFile(filename.c_str(), std::ios::binary);

		// If input file cannot be opened, exit program
		if (!inputFile.is_open())
			throw "Unable to open input file";
	
		// Read in entire file and close input stream
		std::stringstream buf;
		buf << inputFile.rdbuf();
		std::string fileData(buf.str());
		inputFile.close();

		switch(op) {
			case 'c': {
				std::cout << "Compressing file " << filename << std::endl;

				compress(fileData, std::back_inserter(compressed));			// Compress file string
				std::string binStr = variableCompress(compressed);			// Build binary string using variable bit lengths
				std::string intStr = binary2IntegerString(binStr);			// Build integer string from binary string
				filename.append(".lzw2");									// Add ".lzw" to filename
				std::ofstream outFile(filename.c_str(), std::ios::binary);	// Open output file stream
				outFile << intStr;											// Write to file stream
				outFile.close();											// Close file stream
			
				std::cout << "Compressed file saved as " << filename << std::endl;

				break;
			}
			case 'e': {
				std::cout << "Expanding compressed file " << filename << std::endl;

				variableExpand(compressed, fileData);						 
																			// length string bit patters
				std::string decompressed = decompress(compressed.begin(), compressed.end());  // Decompress
				
				filename.erase(filename.length() - 5, 5);					// Remove ".lzw" from filename
				filename.append("2M");										// Add ".2" to filename

				std::ofstream out(filename.c_str(), std::ios::binary);		// Open output file stream
				out << decompressed;										// Write to file stream
				out.close();												// Close file stream
			
				std::cout << "Expanded file saved as " << filename << std::endl;

				break;			
			}
			default: {
				throw "USAGE: ./lzw435 OPERATION (c/e) FILENAME";		 
			}
		}
	} catch(char const *err) {
		std::cout << "The library threw an exeption: " << err << std::endl;
	}
	return 0;
}
