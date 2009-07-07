#include "hubnicklist.h"
#include <QStringList>
#include <QTime>

void HubNickList::slot_quit(QString data)
{
    UserInfo user;
    user.username = data;

    m_mutex.lock(); // Locking to remove
        list.removeOne(user);
    m_mutex.unlock(); // Unlocking after remove
    sortAndUpdate(); // List is complete, updating
}
void HubNickList::slot_myinfo(QString data)
{
    QStringList split;
    QStringList split_exp;
    QString str;
    QRegExp rx("^(\\S+)\\s([^<]+)?(<.*>)?$");
    QChar c;
    QTime t;
    UserInfo user;
    int pos;
    QList<UserInfo>::iterator it;

    split = data.split("$");
    if(split.size() < 5) return; // Incorrect data
    str = split.at(0);
    pos = rx.indexIn(str);
    if(pos > -1){
        split_exp = rx.capturedTexts();
        user.username = split_exp.at(1);
        user.description = split_exp.at(2);
        user.client = split_exp.at(3);
        user.mode = "";
    }
    else
    {
        split_exp = str.split(" ");
        if(split_exp.size() < 2) return; // Incorrect data
        user.username = split_exp.at(0);
        user.description = split_exp.at(1);
        user.client = "";
        user.mode = "";
    }
    user.email = split.at(1);
    str = split.at(2); // Connection type section
    c = str.at(str.size()-1);
    str.remove(str.size()-1, 1);
    user.connection = str;
    user.mode = c;
    user.sharesize = split.at(4).toULongLong();

    m_mutex.lock();
        it = qFind(list.begin(), list.end(), user); // Looking for element

        if(it == list.end()) // Element is not in the list yet
        {
            list << user;
        }
        else
        {
            (*it) = user; // Updating present user
        }
    m_mutex.unlock();
    if(isMy == true || user.username == m_username) // Last MyINFO is my
    {
        isMy = true;
        sortAndUpdate(); // List is complete, updating
    }
}
HubNickList::HubNickList(QString username) : m_username(username) {
    isMy = false;
}
bool HubNickList::LessThan(const UserInfo &s1, const UserInfo &s2)
{
    return s1.username.toLower() < s2.username.toLower();
}
void HubNickList::sortAndUpdate()
{
    emit signal_list_about_to_be_changed();
        m_mutex.lock();
            qSort(list.begin(), list.end(), HubNickList::LessThan);
        m_mutex.unlock();
    emit signal_list_changed();
}
int HubNickList::size()
{
    m_mutex.lock();
    int s = list.size();
    m_mutex.unlock();
    return s;
}
UserInfo HubNickList::at(int i)
{
    m_mutex.lock();
    UserInfo s = list.at(i);
    m_mutex.unlock();
    return s;
}
