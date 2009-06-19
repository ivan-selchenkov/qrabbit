#include "searchmanager.h"

SearchManager::SearchManager(QObject* parent, QList<DirsTree> & _t, SearchItem _search_item): QObject(parent), tree(_t)
{
    search_item = _search_item;
    list = _search_item.data.split(QRegExp("\\W+"), QString::SkipEmptyParts);
    QString pattern;
    foreach(QString str, list)
        pattern.append(str + "|");
    if(pattern.size() > 0)
        pattern.remove(-1,1);
    exp.setPattern(QString("(%1)").arg(pattern));
    exp.setCaseSensitivity(Qt::CaseInsensitive);
    //qDebug() << "SearchManager(" << search_item.data << ")";
}
SearchManager::~SearchManager()
{
    //qDebug() << "~SearchManager()";
}
void SearchManager::run()
{
    //qDebug() << "SearchManager->run()";
    int i;

    for(i=0; i<tree.size(); i++)
        searchDirectory(tree[i]); // searching in subdirectories
}
void SearchManager::searchDirectory(DirsTree &realTree)
{
    QString cur;
    bool send;

    for(int i=0; i<realTree.childDirs.size(); i++)        
        searchDirectory(realTree.childDirs[i]); // searching in subdirectories

    if(search_item.type == SearchItem::FOLDER)
    {
        FileInfo fi;
        fi.dir = realTree.current;
        fi.isDir = true;
        fi.relativePath = realTree.relativePath;

        qRegisterMetaType<FileInfo>("FileInfo");
        qRegisterMetaType<SearchItem>("SearchItem");
        emit signal_search_result(fi, search_item);
        return;
    }

    for(int i=0; i<realTree.files.size(); i++) // analyzing file
    {
        send = false;
        switch(search_item.type) // checking file type
        {
            case SearchItem::TTH:
                if(realTree.files[i].TTH == list.at(1))
                    send = true;
            case SearchItem::AUDIO:
                if(realTree.files[i].filename.contains(QRegExp("\.(aac|aif|flac|iff|m3u|mid|midi|mp3|mpa|ogg|ra|ram|wav|wma)$", Qt::CaseInsensitive)))
                    if(realTree.files[i].filename.contains(exp)) send = true;
            break;
            case SearchItem::ARCHIVE:
                if(realTree.files[i].filename.contains(QRegExp("\.(7z|deb|gz|pkg|rar|sea|sit|sitx|zip)$", Qt::CaseInsensitive)))
                    if(realTree.files[i].filename.contains(exp)) send = true;
            break;
            case SearchItem::BIN: // executable
                if(realTree.files[i].filename.contains(QRegExp("\.(app|bat|cgi|com|exe|jar)$", Qt::CaseInsensitive)))
                    if(realTree.files[i].filename.contains(exp)) send = true;
            break;
            case SearchItem::DOCUMENT:
                if(realTree.files[i].filename.contains(QRegExp("\.(doc|docx|rtf|txt|odt|ods|odp|xls)$", Qt::CaseInsensitive)))
                    if(realTree.files[i].filename.contains(exp)) send = true;
            break;
            case SearchItem::IMAGE:
                if(realTree.files[i].filename.contains(QRegExp("\.(3dm|3dmf|ai|blend|bmp|cpt|cr2|drw|dwg|dxf|eps|gif|indd|jpg|mng|pct|pdf|png|ps|psd|svg|tif)$", Qt::CaseInsensitive)))
                    if(realTree.files[i].filename.contains(exp)) send = true;
            break;
            case SearchItem::ISO:
                if(realTree.files[i].filename.contains(QRegExp("\.(000|iso|dmg|nrg|toast|vcd)$", Qt::CaseInsensitive)))
                    if(realTree.files[i].filename.contains(exp)) send = true;
            break;
            case SearchItem::VIDEO:
                if(realTree.files[i].filename.contains(QRegExp("\.(3g2|3gp|asf|asx|avi|flv|mkv|mov|mp4|mpg|qt|rm|swf|vob|wmv)$", Qt::CaseInsensitive)))
                    if(realTree.files[i].filename.contains(exp)) send = true;
            break;
            default:
                if(realTree.files[i].filename.contains(exp)) send = true;
            break;
        }
        if(send) // if filename is suitable
        {
            qRegisterMetaType<FileInfo>("FileInfo");
            qRegisterMetaType<SearchItem>("SearchItem");
            emit signal_search_result(realTree.files[i], search_item);
        }
    }
}
