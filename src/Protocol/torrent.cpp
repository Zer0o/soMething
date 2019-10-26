#include "torrent.h"

#include <iostream>
#include <stack>

void TorrentUnit::print(TorrentUnit *head)
{
    std::stack<TorrentUnit*> ts;
    TorrentUnit *cur = NULL;
    ts.push(head);
    while (!ts.empty())
    {
        cur = ts.top();
        ts.pop();

        if (cur->unit_type == TorrentUnitType::INTEGER)
        {
            std::cout << "Int/" << cur->int_value << std::endl;
        }
        else if (cur->unit_type == TorrentUnitType::STRING)
        {
            std::cout << "Str/" << cur->str_value << std::endl;
        }
        else if (cur->unit_type == TorrentUnitType::DICT)
        {
            std::cout << "Dict" << std::endl;
            if (cur->unit_value) ts.push(cur->unit_value);
            if (cur->key_value) ts.push(cur->key_value);
        }
        else if (cur->unit_type == TorrentUnitType::LIST)
        {
            std::cout << "List" << std::endl;
            while (!cur->tkl.empty())
            {
                TorrentUnit *tmp = cur->tkl.back();
                cur->tkl.pop_back();
                ts.push(tmp);
            }
        }
        else if (cur->unit_type == TorrentUnitType::DICTLIST)
        {
            std::cout << "DictList" << std::endl;
            while (!cur->tkl.empty())
            {
                TorrentUnit *tmp = cur->tkl.back();
                cur->tkl.pop_back();
                ts.push(tmp);
            }
        }
    }
}

TorrentUnit* Torrent::parse(std::string torrent_link)
{
    std::stack<TorrentUnit*> ts, tms;
    TorrentUnit *head = new TorrentUnit(TorrentUnitType::LIST);
    ts.push(head);

    TorrentUnit *cur = NULL;
    bool oneEnd = true;
    bool strBegin = false;
    for (auto &ch : torrent_link)
    {
        if (oneEnd)
        {
            if (ch == 'i')
            {
                cur = new TorrentUnit(0);
                oneEnd = false;
            }
            else if (ch == 'l')
            {
                cur = new TorrentUnit(TorrentUnitType::LIST);
                ts.push(cur);
            }
            else if (ch == 'd')
            {
                cur = new TorrentUnit(TorrentUnitType::DICTLIST);
                ts.push(cur);
            }
            else if (ch >= '0' && ch <= '9')
            {
                cur = new TorrentUnit(TorrentUnitType::STRING);
                cur->int_value = ch - '0';
                oneEnd = false;
            }
        }
        else if (ch >= '0' && ch <= '9' 
            && !strBegin)
        {
            cur->int_value = cur->int_value * 10 + (ch - '0');
        }
        else if (ch == 'e' && cur->unit_type != TorrentUnitType::STRING)
        {
            oneEnd = true;
            
            if (cur->unit_type == TorrentUnitType::INTEGER)
            {
                ts.push(cur);
            }
        }
        else if (ch == ':' && !strBegin)
        {
            strBegin = true;
        }
        else
        {
            cur->str_value += ch;
            if (cur->str_value.size() == cur->int_value)
            {
                oneEnd = true;
                ts.push(cur);
                strBegin = false;
            }
        }
    }

    while (!ts.empty())
    {
        cur = ts.top();
        
        ts.pop();

        if (cur->unit_type == TorrentUnitType::LIST)
        {
            while(!tms.empty())
            {
                cur->tkl.push_back(tms.top());
                tms.pop();
            }
        }
        else if (cur->unit_type == TorrentUnitType::DICTLIST)
        {
            while (!tms.empty())
            {
                if (tms.size() < 2) break;
                TorrentUnit *t1 = tms.top();
                tms.pop();
                TorrentUnit *t2 = tms.top();
                tms.pop();
                TorrentUnit *nt = new TorrentUnit(TorrentUnitType::DICT);
                nt->add(t1, t2);
                cur->tkl.push_back(nt);
            }
        }

        tms.push(cur);
    }

    TorrentUnit::print(head);

    return head;
}