#pragma once

#include <list>
#include <map>
#include <string>

enum TorrentUnitType
{
    INTEGER = 0,
    STRING,
    LIST,
    DICT,
    DICTLIST,
};

class TorrentUnit
{
public:
    TorrentUnit(TorrentUnitType t) : unit_type(t), key_value(NULL), unit_value(NULL) {}
    TorrentUnit(int v) : unit_type(TorrentUnitType::INTEGER), int_value(v) {}
    //TorrentUnit(std::string s, int len) : unit_type(TorrentUnitType::STRING), str_value(str_value), int_value(len) {}
    ~TorrentUnit() {}

    void add(TorrentUnit *k, TorrentUnit* v)
    {
        unit_type = TorrentUnitType::DICT;
        key_value = k;
        unit_value = v;
    }

    void add(TorrentUnit *u)
    {
        unit_type = TorrentUnitType::LIST;
        tkl.push_back(u);
    }

    static void print(TorrentUnit *head);

    TorrentUnitType unit_type;

    TorrentUnit *key_value;
    TorrentUnit *unit_value;
    std::string str_value;
    unsigned int int_value; //unit_type == INTEGER, int value; unit_type == STRING, str len
    std::list<TorrentUnit*> tkl;
};

class Torrent
{
public:
    static TorrentUnit* parse(std::string torrent_link);
};