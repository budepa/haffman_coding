#define _CRT_SECURE_NO_WARNINGS
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <string>
#include <queue>
#include <unordered_map>

using namespace std;

class Uzel {
public:
    string key;
    uint64_t size;
    Uzel* R;
    Uzel* L;

    bool operator() (const Uzel& x, const Uzel& y)
    {
        return x.size > y.size;
    }

    Uzel(const string& value = "", uint64_t amount = 0,
        Uzel* left = NULL, Uzel* right = NULL)
    {
        key = value;
        size = amount;
        L = left;
        R = right;
    }


    Uzel* join(Uzel x)
    {
        return new Uzel(x.key + key, x.size + size, new Uzel(x), this);
    }
};

Uzel* builder(priority_queue<Uzel, vector<Uzel>, Uzel> leafs)
{
    while (leafs.size() > 1)
    {
        Uzel* n = new Uzel(leafs.top());

        leafs.pop();
        leafs.push(*n->join(*new Uzel(leafs.top())));
        leafs.pop();
    }
    return new Uzel(leafs.top());
}

void huffmanCodes(Uzel* root, string  code, unordered_map < string, string >* huffmanCode)
{
    if (root == nullptr)
        return;
    if (!root->L && !root->R)
    {
        (*huffmanCode)[root->key] = code;
    }

    huffmanCodes(root->L, code + "0", huffmanCode);
    huffmanCodes(root->R, code + "1", huffmanCode);
}

void coder(const char* input_name = "text.txt",
    const char* output_name = "encode.txt") {
    uint64_t* alfabet = new uint64_t[256];
    for (int i = 0; i < 256; i++) {
        alfabet[i] = 0;
    }
    FILE* input_file = fopen(input_name, "rb");
    if (input_file == nullptr) {
        throw invalid_argument("missing file");
    }

    unsigned char character = 0;
    while (!feof(input_file))
    {
        character = fgetc(input_file);
        if (!feof(input_file))
        {
            alfabet[character]++;
        }
    }

    fclose(input_file);

    priority_queue<Uzel, vector<Uzel>, Uzel> leafs;
    for (int i = 0; i < 256; i++)
    {
        
        if (alfabet[i] != 0) {
            string s(1, static_cast<char>(i));
            Uzel new_leaf(s, alfabet[i]);
            leafs.push(new_leaf);
        }
    }

    Uzel* tree = builder(leafs);

    unordered_map<string, string> huffmanCode;
    huffmanCodes(tree, "", &huffmanCode);

    std::cout << "codes:\n";
    for (auto pair : huffmanCode)
    {
        cout << pair.first << " " << pair.second << '\n';
    }

    FILE* output_file = fopen(output_name, "wb +");
    input_file = fopen(input_name, "rb");

    character = 0;
    unsigned char k = 0;
    unsigned int len = 0;

    unsigned int bit_len = 0;
    unsigned char letter = 0;
    char count_letters = leafs.size();
    fputc(count_letters, output_file);

    for (int i = 0; i < 256; i++)
    {
        if (alfabet[i] != 0)
        {
            fputc(static_cast<char>(i), output_file);
            fwrite(reinterpret_cast<const char*>(&alfabet[i]),
                sizeof(uint64_t), 1, output_file);
        }
    }

    while (!feof(input_file))
    {
        character = fgetc(input_file);
        if (!feof(input_file)) {
            std::string s(1, character);
            if (bit_len + huffmanCode[s].length() <= 8)
            {
                for (int i = 0; i < huffmanCode[s].length(); i++)
                {
                    letter = letter << 1 | (huffmanCode[s][i] - '0');
                }
                bit_len += huffmanCode[s].length();
            }
            else {
                for (int i = 0; i < 8 - bit_len; i++)
                {
                    letter = letter << 1 | (huffmanCode[s][i] - '0');
                }
                if (huffmanCode[s].length() - (8 - bit_len) >= 8)
                {
                    int i = 8 - bit_len;
                    while (i + 7 < huffmanCode[s].length())
                    {
                        k = 0;

                        for (int j = 0; j < 8; j++)
                        {
                            k = k << 1 | (huffmanCode[s][i + j] - '0');
                        }

                        i += 8;
                        fputc(letter, output_file);
                        letter = k;
                    }

                    k = 0;
                    len = 0;

                    for (int j = i; j < huffmanCode[s].length(); j++)
                    {
                        k = k << 1 | (huffmanCode[s][j] - '0');
                        len++;
                    }
                }
                else
                {
                    len = 0;
                    for (int i = 8 - bit_len; i < huffmanCode[s].length(); i++)
                    {
                        k = k << 1 | (huffmanCode[s][i] - '0');
                        len++;
                    }
                }

                bit_len = 8;
            }

            if (bit_len == 8)
            {
                fputc(letter, output_file);

                letter = k;
                bit_len = len;
                k = 0;
                len = 0;
            }
        }
        else if (bit_len < 8)
        {
            letter = letter << (8 - bit_len);
            fputc(letter, output_file);
        }
    }

    fclose(input_file);
    fclose(output_file);

}

float compressRatio(const char* input_name = "text.txt", const char* output_name = "encode.txt") {
    uint64_t file_full_size = 0;
    uint64_t compress_size = 0;

    struct stat sb {};
    struct stat se {};

    if (!stat(input_name, &sb))
    {
        file_full_size = sb.st_size;
    }
    else
    {
        perror("STAT");
    }
    if (!stat(output_name, &se))
    {
        compress_size = se.st_size;
    }
    else
    {
        perror("STAT");
    }

    return (compress_size + 0.0) / file_full_size;
}

int main() {
    coder();
    cout << "Compress ratio is: " << compressRatio() << endl;
}