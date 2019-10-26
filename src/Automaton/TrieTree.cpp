#include "TrieTree.h"

#include <iostream>
#include <queue>

TrieNode *TrieNode::getChild(wchar_t &ch)
{
    auto iter = next.find(ch);
    if (iter == next.end())
    {
        return NULL;
    }

    return iter->second;
}

void TrieNode::addChild(wchar_t &ch, TrieNode *node)
{
    next[ch] = node;
}

void Automation::addWord(std::wstring &word)
{
    int update_idx = 0;
    {
        std::unique_lock<std::mutex> lk(_mutex);
        update_idx = getOldIndex();
        //std::wcout << "addWord idx = " << update_idx;
        cv.wait(lk, [this, update_idx] {
            //std::wcout << "addWord count " << _h[update_idx & 0x1]._count << std::endl;
            return _h[update_idx & 0x1]._count == 0;});
    }

    TrieNode *head = _h[update_idx & 0x1].head, *cur = head;
    *(_h[update_idx & 0x1].head) = *(_h[(update_idx+1) & 0x1].head);
    for(auto &ch : word)
    {
        TrieNode *child = cur->getChild(ch);
        
        if (child)
        {
            cur = child; 
        }
        else
        {
            TrieNode *node = new TrieNode(ch);
            cur->addChild(ch, node);
            cur = node;
        }
    }

    if (cur)
    {
        cur->setEnd();
        cur->setWord(word);
    }

    buildFail(head);

    {
        std::unique_lock<std::mutex> lk(_mutex);
        using_index = update_idx;
    }
}

void Automation::delWord(std::wstring &word)
{
    int update_idx = 0;
    {
        std::unique_lock<std::mutex> lk(_mutex);
        update_idx = getOldIndex();
        //std::wcout << "delWord idx = " << update_idx;
        cv.wait(lk, [this, update_idx] {
            //std::wcout << "delWord count " << _h[update_idx & 0x1]._count << std::endl;
            return _h[update_idx & 0x1]._count == 0;});
    }

    TrieNode *head = _h[update_idx & 0x1].head, *cur = head;
    *(_h[update_idx & 0x1].head) = *(_h[(update_idx+1) & 0x1].head);
    for (auto &ch : word)
    {
        TrieNode *child = cur->getChild(ch);

        if (!child)
        {
            return ;
        }

        cur = child;
    }

    cur->setEnd(false);

    buildFail(head);

    {
        std::unique_lock<std::mutex> lk(_mutex);
        using_index = update_idx;
    }
}

void Automation::buildFail(TrieNode *head)
{
    std::queue<TrieNode*> q;

    for (auto &trienode : head->next)
    {
        q.push(trienode.second);
        trienode.second->fail = head;
    }

    while (!q.empty())
    {
        TrieNode *cur = q.front();
        q.pop();
        
        for (auto &iter : cur->next)
        {
            TrieNode *curFail = cur->fail, *failNode = NULL;
            while (curFail)
            {
                failNode = curFail->getChild(iter.second->_ch);
                if (failNode)
                {
                    break;
                }
                
                curFail = curFail->fail;
            }

            iter.second->fail = failNode ? failNode : head;

            q.push(iter.second);
        }
    }
}

void Automation::print()
{
#if 0
    std::queue<TrieNode*> q;
    q.push(head);

    while(!q.empty())
    {
        TrieNode *tmp = q.front();
        q.pop();
        
        std::wcout << "ch = " << tmp->_ch;
        std::wcout << ", p = " << static_cast<const void *>(tmp);
        std::wcout << ", fp = " << static_cast<const void *>(tmp->fail);
        std::wcout << ", end = " << tmp->_end << std::endl; 

        for (auto &iter : tmp->next)
        {
            q.push(iter.second);
        }
    }
#endif
}

std::vector<std::wstring> Automation::ac(std::wstring &sentence)
{
    std::vector<std::wstring> vec;

    TrieNode *cur = NULL, *next = NULL, *head = NULL;

    int idx = 0;
    {
        std::unique_lock<std::mutex> lock(_mutex);
        idx = getUsingIndex();
        head = _h[idx & 0x1].head;
        _h[idx & 0x1]._count++;
        //std::wcout << "ac idx = " << idx;
        //std::wcout << ", count = " << _h[idx&0x1]._count << std::endl;
        cv.notify_all();
    }

    cur = head;

    for (size_t i = 0; i < sentence.size(); ++i)
    {
        auto &ch = sentence[i];

        while ((next = cur->getChild(ch)) == NULL && cur != head)
        {
            cur = cur->fail;
        }

        cur = next ? next : head;

        TrieNode *tmp = cur;
        while (tmp != head)
        {
            if (tmp->end())
            {
                vec.push_back(tmp->getWord());
            }
            tmp = tmp->fail;
        }
    }

    {
        std::unique_lock<std::mutex> lock(_mutex);
        _h[idx & 0x1]._count--;
        //std::wcout << "ac idx = " << idx;
        //std::wcout << ", count = " << _h[idx&0x1]._count << std::endl;
        cv.notify_all();
    }
    
    return vec;
}