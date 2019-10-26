#include "FST.h"

void FST::add(std::wstring &text, int val)
{
    FSTNode *cur = _head, *next = NULL;
    int left = val, add = 0;
    for (size_t i = 0; i < text.size(); ++i)
    {
        next = NULL;
        for (auto node : cur->next)
        {
            if (node->_ch == text[i])
            {
                next = node;
            }
            else
            {
                node->_val += add;
            }
        }

        if (!next)
        {
            FSTNode *nn = new FSTNode(text[i], left);
            cur->next.push_back(nn);
            left = 0;
            cur = nn;
        }
        else
        {
            if (next->_val <= val)
            {
                left = val - next->_val;
            }
            else
            {
                add = next->_val - val;
                next->_val = val;
                left = 0;
            }
            
            cur = next;
        }
    }
}

int FST::search(std::wstring &text)
{
    FSTNode *cur = _head, *next = NULL;
    int val;
    for (size_t i = 0; i < text.size(); ++i)
    {
        next = NULL;
        for (auto node : cur->next)
        {
            if (node->_ch == text[i])
            {
                next = node;
                break;
            }
        }

        if (!next)
        {
            return 0;
        }

        val += next->_val;
        cur = next;
    }

    return val;
}