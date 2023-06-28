#define _CRT_SECURE_NO_WARNINGS
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
        Uzel* left = NULL, Uzel* right = NULL) {
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

Uzel* builder(priority_queue<Uzel, vector<Uzel>, Uzel> tree)\
{
    while (tree.size() > 1)
    {
        Uzel* n = new Uzel(tree.top());

        tree.pop();
        tree.push(*n->join(*new Uzel(tree.top())));
        tree.pop();
    }
    return new Uzel(tree.top());
}


void decoder(const char* input_name = "encode.txt",
    const char* output_name = "decode.txt") {
    uint64_t* alfabet = new uint64_t[256];
    for (int i = 0; i < 256; i++) {
        alfabet[i] = 0;
    }
    FILE* input_file = fopen(input_name, "rb");
    if (input_file == nullptr) {
        throw invalid_argument("missing file");
    }
    unsigned char col = 0;
    unsigned int col_letters = 0;
    col = fgetc(input_file);
    if (!feof(input_file)) {
        col_letters = (unsigned int)col;
    }

    unsigned char character = 0;
    for (int i = 0; i < col_letters; i++)
    {
        character = fgetc(input_file);
        if (!feof(input_file))
        {
            fread(reinterpret_cast<char*>(&alfabet[character]),
                sizeof(uint64_t), 1, input_file);
        }
        else
        {
            throw invalid_argument("decompession failed");
        }
    }

    priority_queue<Uzel, vector<Uzel>, Uzel> tree;
    for (int i = 0; i < 256; i++) {
        if (alfabet[i] != 0) {
            string s(1, static_cast<char>(i));

            Uzel new_leaf(s, alfabet[i]);
            tree.push(new_leaf);
        }
    }
    character = 0;
    Uzel* n = builder(tree);

    FILE* output_file = fopen(output_name, "wb+");

    Uzel* nodes = n;
    unsigned char letter = 0;
    while (!feof(input_file))
    {
        character = fgetc(input_file);
        if (!feof(input_file))
        {
            for (int i = 7; i > -1; i--) {
                if (((character >> i) & 1) == 1)
                {
                    if (nodes->R == NULL)
                    {
                        letter = nodes->key[0];
                        if (alfabet[letter] > 0)
                        {
                            alfabet[letter]--;
                            fputc(letter, output_file);
                            nodes = n->R;
                        }
                    }
                    else
                    {
                        nodes = nodes->R;
                    }
                }
                else if (((character >> i) & 1) == 0)
                {
                    if (nodes->L == NULL)
                    {
                        letter = nodes->key[0];
                        if (alfabet[letter] > 0)
                        {
                            alfabet[letter]--;
                            fputc(letter, output_file);
                            nodes = n->L;

                        }
                    }
                    else
                    {
                        nodes = nodes->L;
                    }
                }
            }
        }
    }

    fclose(input_file);
    fclose(output_file);
}


unsigned int checker(const char* before_name = "text.txt",
    const char* after_name = "decode.txt") {
    unsigned int same = 0;
    FILE* before_file = fopen(before_name, "r");
    FILE* after_file = fopen(after_name, "r");

    unsigned char after_l = 0;
    unsigned char before_l = 0;
    while (!feof(after_file) && !feof(before_file))
    {
        after_l = fgetc(after_file);
        before_l = fgetc(before_file);
        if (!feof(after_file) && !feof(before_file))
        {
            if (after_l != before_l)
            {
                same++;
            }
        }
    }

    while (!feof(after_file))
    {
        after_l = fgetc(after_file);
        if (!feof(after_file))
        {
            same++;
        }
    }

    while (!feof(before_file))
    {
        before_l = fgetc(before_file);
        if (!feof(before_file))
        {
            same++;
        }
    }
    fclose(after_file);
    fclose(before_file);
    return same;
}

int main() {
    decoder();
    if (!checker())
    {
        cout << "match" << endl;
    }
    else
    {
        cout << "wrong match" << endl;
    }
}