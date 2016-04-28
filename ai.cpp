#include "filter.h"
#include <iterator>
#include <vector>
#include <iterator>
#include <time.h>
#include <map>
#include "console.h"
#include "Pos.h"
#include "sdk.h"
#include "filter.h"
#include <iterator>
#include <vector>
#include <iterator>
#include <string>
#include <sstream>
#include <iostream>
using namespace std;

/*
2016.04
author:gtk
generalization : Event oriented Priority Queue.
Artificial Intelligence -Version 2.0
*/
#define LOCAL
#ifdef LOCAL
#include <fstream>
static ofstream fout("log.txt");
#endif // LOCAL

//ǰ������
class Commander;
class Soldier;

Console* console;
Commander* commander;
enum Behavior :int { VsMonster, VsEnemy, RunAway, DefendBase, Mining, AttackBase, MixedFight, LevelUp, Free }; //ģ��Ӣ����Ϊ
enum HeroType :int { Hammerguard, Master, Berserker, Scouter };
bool operator< (const Pos&, const Pos&);//Ϊ��ʹ��std::map<Pos,*>,��û��ʵ�ʵ����塣
string behaviorList[] = { "VsMonster", "VsEnemy", "RunAway", "DefendBase", "Mining", "AttackBase", "MixedFight", "LevelUp", "Free" };//Ϊ�˵���,��ʵ������

                                                                                                                                     /*
                                                                                                                                     Log for Debug
                                                                                                                                     */
class Log
{
public:
    Log(const string& str, ostream & = cout);
    ~Log();
private:
    int startTime;
    string str;
    ostream &os;
};

/*
the Mine Unit
*/
class Mine
{
public:
    Mine()
    {
    }

    PUnit* mons;
    vector<PUnit*> enemy;
    vector<PUnit*> soldiers;
    Pos pos;
    int energy;
};

/*
the Base Unit
*/
class Base
{
public:
    Base()
    {
    }

    PUnit* ptr;
    int hp;
    Pos pos;
};

/*
Area Declaration
used to detect the specfic events happened in this Aear
*/
//  as for the Commander itself, it doesn't know the exact field of where the enemy
//is, which is likely to cause some misplays. So the Area is to modify this bug.
class Area
{
public:
    Area()
    {
    }
    PUnit* discoverer; //������ĺ��������Ա
    int turns; //���غ���
    vector<PUnit*> soldiers;    //��������������Ӣ��
    vector<PUnit*> enemy;   //���������
    vector<PUnit*> monster;//���������
    vector<Mine*> mines;
};

class Event
{
public:
    Event()
    {
        pos.clear();
        bPreemptive = false;
    }

    vector<Pos> pos;    //�¼�ָ��λ��
    PUnit* signaler;    //�¼�������
    Behavior type;  //�¼���Ϊ
    int priority;   //���ȶ�
    int turns;  //�¼��غ�
    bool bPreemptive;   //�Ƿ���ռ

    PUnit* findUnitbyPos(Pos pos);
};

/*
extension of PUnit Hero
*/
class Soldier
{
public:
    Soldier() :ptr(nullptr), target(nullptr)
    {
    }

    PUnit* ptr;
    PUnit* target;
};

/*
ս��ָ�ӹ� Declaration
Hopes to hold the global infomation of the battle.
*/
class Commander
{
public:
    const int GeneralView = 144;
    const int ObseverView = 400;
    const int heroTypeNum = 4;
    int curHeroNum;
    Commander();
    ~Commander();

    void start();       //The entrance of the Commander

#ifndef LOCAL
private:
#endif // !LOCAL
    void analysis();    //the Analysis module of the whole battle
    void initHeros();
    void acquirePara();
    void explore();
    void assigment(); //the Assignment module of the whole battle
    void levelUp();
    void buyNewHero();
    void discover();
    void execution(); //the Execute module
    void unique();
    void run();
    /*
    the specific strategy part
    */
    void strgLoopMonster(); //ѭ����Ұ
    void strgAttackBase();  //�����������
    void strgDefenseBase(); //�ؼ�
    void strgAttackHero();  //��������Ӣ��
    void strgRunAway();     //����
    void strgFindMonster(); //Ѱ�ҹ���
                            //design something else
    string heroList[4];    //Ӣ�������б�
    map<int, PUnit*> idToSoldier;
    map<Pos, Mine*> posToMine;
    vector<PUnit*> soldiers;    //��ǰ�ҷ�Ӣ��
    vector<PUnit*> enemy;   //��ǰ����
    vector<PUnit*> monster;//��ǰ����
    vector<PUnit*> allUnits;        //���е�λ
    vector<Area*> fields;    //ϸ��������Ϣ
    vector<Event*> eventQuene;  //�¼�����
    Base *emenyBase;
    Base *myBase;
};

void player_ai(const PMap &map, const PPlayerInfo &info, PCommand &cmd)
{
#ifdef LOCAL
    Log log = Log("znt.txt", fout);
#endif // LOCAL
    console = new Console(map, info, cmd);
    if (console->round() == 0)
        commander = new Commander();

    commander->start();
#ifdef LOCAL
    fout << "Ӣ����Ϣ: " << endl;
    for (auto ptr : commander->idToSoldier)
    {
        if (ptr.second != nullptr) {
            fout << "����: " << ptr.second->name << "   ";
            fout << "id: " << ptr.first << "   ";
            fout << "pos: " << ptr.second->pos << "   " << endl;
        }
    }
    fout << "����Ϣ: " << endl;
    for (auto ptr : commander->posToMine)
    {
        fout << "λ��: " << ptr.first << "   ";
        fout << "����: " << ptr.second->energy << "   ";
        if (ptr.second->soldiers.size() == 0)
            fout << "Ӣ��: ��ǰ��Ӣ����Ϣ" << "   ";
        else
        {
            for (auto f : ptr.second->soldiers)
            {
                fout << "����Ӣ������: " << f->name << "   ";
                fout << "����Ӣ��id: " << f->id << "   ";
                fout << "����Ӣ��pos: " << f->pos << "   " << endl;
            }
        }
        if (ptr.second->enemy.size() == 0)
            fout << "����: ��ǰ�޵�����Ϣ" << "   ";
        else
        {
            for (auto f : ptr.second->soldiers)
            {
                fout << "���ڵ�������: " << f->name << "   ";
                fout << "���ڵ���id: " << f->id << "   ";
                fout << "���ڵ���pos: " << f->pos << "   " << endl;
            }
        }
        if (ptr.second->mons == nullptr)
            fout << "δ���ָÿ�" << "   " << endl;
        else
            fout << "���ָÿ�(pos): " << ptr.first << endl;
    }
    fout << "��ǰ�ҷ�����Ѫ��:" << commander->myBase->hp << "\t" << "�ҷ�����λ��: " << commander->myBase->pos << endl;
    if (commander->emenyBase->ptr == nullptr)
        fout << "δ���ֵз�����" << endl;
    else
        fout << "��ǰ�з�����Ѫ����" << commander->emenyBase->hp << endl;
    fout << "��ǰ��������:" << commander->monster.size() << endl;
    fout << "��ǰӢ������:" << commander->soldiers.size() << endl;
    fout << "��ǰ��������:" << commander->enemy.size() << endl;
    fout << "��ǰ���е�λ����:" << commander->allUnits.size() << endl;
    fout << "[explore ģ��]" << endl;
    if (commander->fields.size() == 0)
        fout << "��explore����" << endl;
    for (int i = 0; i < commander->fields.size(); i++)
    {
        Area* a = commander->fields.at(i);
        fout << "��" << i + 1 << "��xplore����:\t" << endl;
        fout << "�������Ӣ����Ϣ(����):" << a->discoverer->name << "\t(id):" << a->discoverer->id << endl;
        fout << "���ֻغ�: " << a->turns << endl;
        fout << "��������Ұ������: " << a->monster.size() << endl;
        fout << "�������ڵ�������: " << a->enemy.size() << endl;
        fout << "���������ҷ�Ӣ��[Detail]: " << endl;
        for (auto ptr : a->soldiers)
        {
            fout << "����: " << ptr->name << "   ";
            fout << "id: " << ptr->id << "   ";
            fout << "pos: " << ptr->pos << "   " << endl;
        }
    }
    fout << "�ҷ����غ�ӵ�н�Ǯ: " << console->gold() << endl;
    fout << "�ҷ����غ����Ľ�Ǯ: " << console->goldCostCurrentRound() << endl;
    fout << "---------------------�غϽ���-----------------" << endl;
#endif // LOCAL
    delete console;
}

bool operator< (const Pos& a, const Pos& b)
{
    return a.x + a.y < b.x + b.y;
}

Log::Log(const string &_s, ostream &_os) :str(_s), os(_os)
{
    os << str << " " << "start:" << endl;
    startTime = clock();
}

Log::~Log()
{
    os << str << " " << "end, time cost:" << (double)(clock() - startTime) / CLOCKS_PER_SEC << endl;
}

PUnit* Event::findUnitbyPos(Pos pos)
{
    float f = 144.0f;
    PUnit* t = nullptr;
    vector<PUnit*> e = console->enemyUnits();
    for (auto p : e)
        if (p->isHero())
            if (dis2(p->pos, pos) < f)
            {
                f = dis2(p->pos, pos);
                t = p;
            }

    return t;
}

Commander::Commander()
{
    curHeroNum = 0;
    //clear
    fields.clear();
    soldiers.clear();
    allUnits.clear();
    eventQuene.clear();
    //tries to init heros
    initHeros();

    for (int i = 0; i < 100; i++)
        idToSoldier[i] = nullptr;

    //Mine Info
    for (int i = 0; i < 7; i++)
    {
        Mine *mine = new Mine;
        mine->pos = MINE_POS[i];
        mine->energy = -1;
        mine->mons = nullptr;
        mine->enemy.clear();
        mine->soldiers.clear();
        posToMine[mine->pos] = mine;
    }

    //BaseInfo
    emenyBase = new Base;
    emenyBase->hp = 5000;
    emenyBase->ptr = nullptr;
    emenyBase->pos = MILITARY_BASE_POS[console->camp()];
    myBase = new Base;
    myBase->hp = 5000;
    myBase->ptr = nullptr;
    myBase->pos = MILITARY_BASE_POS[1 - console->camp()];
}


void Commander::initHeros()
{
    // don't change
    string h[] = { "Hammerguard", "Master", "Berserker", "Scouter" };
    memcpy(heroList, h, sizeof(h));
    for (auto s : heroList)
    {
        console->chooseHero(s);
        curHeroNum++;
    }
    for (auto ptr : idToSoldier)
        ptr.second = nullptr;
}

void Commander::analysis()
{
    //update the global information (including mySoldier, enemy and the wilds)
    acquirePara();
    //discover the exact fields
    explore();
}

void Commander::acquirePara()
{
    //clear previous
    allUnits.clear();
    enemy.clear();
    soldiers.clear();
    monster.clear();

    //my Para
    vector<PUnit*> units = console->friendlyUnits();
    for (auto ptr : units)
    {
        //update the state of militaryBase
        if (ptr->isBase())
        {
            myBase->ptr = ptr;
            myBase->pos = ptr->pos;
            myBase->hp = ptr->hp;
            allUnits.push_back(ptr);
        }
        else if (ptr->isHero())
        {
            idToSoldier[ptr->id] = ptr;
            soldiers.push_back(ptr);
            allUnits.push_back(ptr);
        }
        else
        {
            fout << "unknownUnit: " << ptr->name;
        }
    }
    //enemy Para
    UnitFilter ft;
    ft.setAvoidFilter("mine");
    vector<PUnit*> emeny = console->enemyUnits(ft);
    for (auto ptr : emeny)
    {
        //update the state of militaryBase
        if (lowerCase(ptr->name) == lowerCase("militaryBase"))
        {
            emenyBase->ptr = ptr;
            emenyBase->hp = ptr->hp;
            allUnits.push_back(ptr);
        }
        else if (ptr->isHero())
        {
            idToSoldier[ptr->id] = ptr;
            emeny.push_back(ptr);
            allUnits.push_back(ptr);
        }
        else if (ptr->isWild())
        {
            idToSoldier[ptr->id] = ptr;
            monster.push_back(ptr);
            allUnits.push_back(ptr);
        }
        else
        {
            fout << "unknownUnit: " << ptr->name;
        }
    }
    //mines & monsters
    UnitFilter filter;
    filter.setTypeFilter("mine");
    vector<PUnit*> neutral = console->enemyUnits(filter);
    for (auto ptr : neutral)
    {
        Mine* mine = new Mine;
        try
        {
            mine = posToMine[ptr->pos];
        }
        catch (const std::exception&)
        {
            cout << "warning: unknown mine " << ptr->pos << " " << ptr->id << endl;
            continue;
        }
        mine->soldiers.clear();
        mine->enemy.clear();
        mine->energy = console->unitArg("energy", "c", ptr);
        UnitFilter nearby;
        nearby.setAvoidFilter("mine");
        nearby.setAreaFilter(new Circle(ptr->pos, GeneralView));
        vector<PUnit*> fs = console->friendlyUnits(nearby);
        vector<PUnit*> es = console->enemyUnits(nearby);
        for (auto e : es)
        {
            if (e->isWild())
            {
                mine->mons = e;
                cout << e->name << endl;
            }
            else if (e->isHero())
                mine->enemy.push_back(e);
        }
        for (auto f : fs)
        {
            bool isMining = false;
            if (f->isHero())
                mine->soldiers.push_back(f);
            if (dis2(f->pos, mine->pos) <= 8 && !isMining)
            {
                isMining = true;
                mine->energy--;
            }
        }
    }
}

void Commander::explore()
{
    fields.clear();
    //get info from the perspective of the hero
    for (int i = 0; i < soldiers.size(); i++)
    {
        Area* area = new Area;
        PUnit* hero = soldiers.at(i);
        UnitFilter field;
        field.setAreaFilter(new Circle(hero->pos, GeneralView));
        vector<PUnit*> fs = console->friendlyUnits(field);
        vector<PUnit*> es = console->enemyUnits(field);
        area->discoverer = hero;
        area->turns = console->round();
        for (auto e : es)
        {
            if (e->isWild())
                area->monster.push_back(e);
            else if (e->isHero())
                area->enemy.push_back(e);
            else if (e->isMine())
            {
                try
                {
                    area->mines.push_back(posToMine[e->pos]);
                }
                catch (const std::exception&)
                {
                    cout << "warning: unknown mine(in discover) " << e->pos << " " << e->id << endl;
                    continue;
                }
            }
            else
                cout << "undiscover unit : " << e->name << endl;
        }
        for (auto f : fs)
            if (f->isHero() && f->id != hero->id)
                area->soldiers.push_back(f);
        fields.push_back(area);
    }
    //try the get observation type
    UnitFilter obserFilter;
    obserFilter.setTypeFilter("Observer");
    vector<PUnit*> obser = console->friendlyUnits(obserFilter);
    for (auto o : obser)
    {
        Area *area = new Area;
        obserFilter.setAreaFilter(new Circle(o->pos, ObseverView));
        vector<PUnit*> ofs = console->friendlyUnits(obserFilter);
        vector<PUnit*> oes = console->enemyUnits(obserFilter);
        area->discoverer = o;
        area->turns = console->round();
        for (auto e : oes)
        {
            if (e->isWild())
                area->monster.push_back(e);
            else if (e->isHero())
                area->enemy.push_back(e);
            else if (e->isMine())
            {
                try
                {
                    area->mines.push_back(posToMine[e->pos]);
                }
                catch (const std::exception&)
                {
                    cout << "warning: unknown mine(in discover.Observer) " << e->pos << " " << e->id << endl;
                    continue;
                }
            }
            else
                cout << "undiscover unit(in Observer) : " << e->name << endl;
        }
        for (auto f : ofs)
            if (f->isHero() && f->id)
                area->soldiers.push_back(f);
        fields.push_back(area);
    }
}



void Commander::start()
{
    //analysis the current situation.
    analysis();
    //assign the mission
    assigment();
    //execute the event in the eventQueue
    execution();
}

void Commander::assigment()
{
    //try to buy new hero
    buyNewHero();
    //try to level up
    levelUp();
    //try to discover event
    discover();
}

void Commander::buyNewHero()
{
    if (curHeroNum >= HERO_LIMIT)
        return;
    int cnt[] = { 0, 0, 0, 0 };
    int type, num;
    for (auto ptr : soldiers)
    {
        if (lowerCase(ptr->name) == lowerCase("Berserker"))
            cnt[Berserker]++;
        else if (lowerCase(ptr->name) == lowerCase("Hammerguard"))
            cnt[Hammerguard]++;
        else if (lowerCase(ptr->name) == lowerCase("Master"))
            cnt[Master]++;
        else if (lowerCase(ptr->name) == lowerCase("Scouter"))
            cnt[Scouter]++;
        else
            cout << "unKnown hero Name: " << ptr->name << endl;
    }
    type = rand() % 4;
    num = cnt[type];
    if (console->gold() - console->goldCostCurrentRound() >= 250 * (1 + num))
    {
        console->chooseHero(heroList[type]);
        curHeroNum++;
    }
}

void Commander::levelUp()
{
    if (curHeroNum <= 6)
        return;

    for (auto ptr : soldiers)
    {
        int level = console->unitArg("level", "c", ptr);
        while (level < HERO_MAX_LEVEL && console->gold() - console->goldCostCurrentRound() >= console->levelUpCost(level))
        {
            if (dis2(ptr->pos, myBase->pos) > LEVELUP_RANGE)
            {
                Event* e = new Event;
                e->signaler = ptr;
                e->type = LevelUp;
                e->priority = 2;
                e->bPreemptive = true;
                eventQuene.push_back(e);
            }
            else
                console->buyHeroLevel(ptr);
        }
    }
}

void Commander::discover()
{
    //try to discover some mission in the area and put the mission to the eventQueue, waiting to be executed in 
    //the execute module
    for (auto a : fields)
    {
        Event* e = new Event;
        e->turns = a->turns;
        if (lowerCase(a->discoverer->name) == lowerCase("Observer"))
        {
            //used as a evaluation or something else?
            continue;
        }
        bool isAtk = true;
        for (auto ptr : a->soldiers)
        {
            if (a->soldiers.size() <= 1)
                break;
            if (ptr->level < 3) {
                isAtk = false;
            }
        }
        if (isAtk)
        {
            e->priority = 4;
            e->signaler = a->discoverer;
            e->type = AttackBase;
        }
        if (a->enemy.size() == 0)
        {
            // if there hasn't got any enemy and there is no mine, set free with priority of zero. 
            // ready to be preempted by any of other Hero Behavior.
            if (a->mines.size() == 0)
            {
                e->priority = 0;
                e->signaler = a->discoverer;
                e->type = Free;
            }
            for (auto m : a->mines) //if this area has a mine
            {
                if (m->mons == NULL) {
                    cout << "the Object-monster near the Mine is NULL. upper architecture are wrong!" << endl;
                    continue;
                }
                e->priority = 1;
                e->signaler = a->discoverer;
                if (!m->mons->findBuff("Reviving"))
                {
                    e->type = VsMonster;
                    e->pos.push_back(m->mons->pos);
                }
                else
                {
                    e->type = Mining;
                    e->pos.push_back(m->pos);
                }
            }
        }
        //if there exist emeny
        else
        {
            for (auto ptr : a->enemy)
                e->pos.push_back(ptr->pos);     // maybe not good, need to get the PUnit not the pos to identity the complete info of the Enemy?
            e->signaler = a->discoverer;
            e->priority = 3;
            e->type = VsEnemy;
        }
        eventQuene.push_back(e);
    }
}

void Commander::execution()
{
    //parse the eventQueue & ensure the command of each hero is unque
    unique();
    //run the final eventQueue
    run();
}

void Commander::unique()
{
    vector<PUnit*> unHandle;
    for (auto e : eventQuene)
        if (e->turns < console->round())
            unHandle.push_back(e->signaler);
    if (unHandle.size() == 0)
    {
        for (auto hero : soldiers)
        {
            int maxPriority = -1;
            for (auto e : eventQuene)
            {
                if (hero->id == e->signaler->id)
                    if (e->priority > maxPriority)
                        maxPriority = e->priority;
            }
            for (vector<Event*>::iterator it = eventQuene.begin(); it != eventQuene.end(); )
            {
                if (maxPriority == -1)
                    cout << "illegal priority: " << (*it)->priority << ". The heroName: " << hero->name << endl;
                if ((*it)->signaler->id == hero->id)
                {
                    if ((*it)->priority < maxPriority)
                        it = eventQuene.erase(it);
                    else
                        it++;
                }
                else {
                    it++;
                }
            }
        }
    }//if there exist some unhandled events 
    else
    {
        vector<PUnit*> preCommand;
        for (auto e : eventQuene)
            if (e->bPreemptive)
                preCommand.push_back(e->signaler);

        if (preCommand.size() != 0)
        {
            for (auto ptr : preCommand) //clear the current command.
                for (vector<Event*>::iterator it = eventQuene.begin(); it != eventQuene.end(); )
                {
                    if (ptr->id == (*it)->signaler->id && (*it)->turns == console->round())
                        it = eventQuene.erase(it);
                    else
                        it++;
                }
        }
        else
        {
            for (vector<Event*>::iterator it = eventQuene.begin(); it != eventQuene.end(); )
            {
                if ((*it)->turns != console->round())
                    it = eventQuene.erase(it);
                else
                    it++;
            }
        }

    }
}

void Commander::run()
{
#ifdef LOCAL
    fout << "---------------------��ǰ�غ�: " << console->round() << "-----------------" << endl;
    fout << "-----------[�¼�ģ��]------------" << endl;
    fout << "���غϷ����¼�������: " << commander->eventQuene.size() << endl;
    for (auto e : commander->eventQuene)
    {
        fout << "������: " << e->signaler->name << endl;
        fout << "�¼��غ�: " << e->turns << endl;
        fout << "�¼�����:��" << behaviorList[e->type] << endl;
        fout << "�¼��Ƿ�ɱ���ռ����" << e->bPreemptive << endl;
    }
#endif // LOCAL
    for (vector<Event*>::iterator it = eventQuene.begin(); it != eventQuene.end(); )
    {
        switch ((*it)->type)
        {
        case VsMonster:
            strgLoopMonster();
            it = eventQuene.erase(it);
            break;
        case VsEnemy:
            //do something here 
            it = eventQuene.erase(it);
            break;
        case LevelUp:
            //do something here
            it = eventQuene.erase(it);
            break;
        case Free:
            strgFindMonster();
            it = eventQuene.erase(it);
            break;
            //....something else
        default:
            it = eventQuene.erase(it);
            break;
        }
    }
}

void Commander::strgAttackBase()
{
    //do something

}

void Commander::strgAttackHero()
{
    //do something

}

void Commander::strgDefenseBase()
{
    //do something

}

void Commander::strgLoopMonster()
{
    for (auto e : eventQuene)
    {
        cout << e->signaler->name << endl;
    }
    for (auto e : eventQuene)
    {
        console->selectUnit(e->signaler);

            cout << e->signaler->name << endl;
            if (dis2(e->pos.at(0), e->signaler->pos) <= e->signaler->range)
            {
                cout << "here" << endl;
                console->attack(e->findUnitbyPos(e->pos.at(0)));
            }
            else
            {
                cout << "there" << endl;
                console->move(e->pos.at(0));
            }
    }
}

void Commander::strgRunAway()
{
    //do something

}

void Commander::strgFindMonster()
{
    for (auto e : eventQuene)
    {
        console->selectUnit(e->signaler);
        int find = 0;
        for (auto pos : e->pos)
            if (pos == Dragon_pos[0])
                find = 1;

        if (find == 0)
            e->pos.push_back(Dragon_pos[0]);

        console->move(e->pos.at(0));
    }
}