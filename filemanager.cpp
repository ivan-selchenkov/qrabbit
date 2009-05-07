#include "filemanager.h"
#include "tth/hashfile.h"

FileManager::FileManager()
{
    //HashFile hf;
    //QString TTH = hf.Go("/home/ivan/Dropbox/QT/libhash_new/libhash_new.pro");

    folders.append(QDir("/home/ivan/Dropbox/"));

    //LoadXML();
    ScanFiles();

}
void FileManager::ScanFiles()
{
    for(int i=0; i<folders.size(); i++)
    {
        DirsTree dir;
        if(!QDir(folders.at(i)).exists()) continue;
        dir.current = folders.at(i);
        scan(dir);
        tree.append(dir);
    }

    SaveXML();
}
void FileManager::scan(DirsTree& node)
{
    FileInfo fi;
    HashFile hf;

    QStringList listFiles = node.current.entryList(QDir::Files);
    foreach(QString file, listFiles)
    {
        fi.Clear();
        fi.filename = file;
        fi.dir = node.current;
        fi.size = QFileInfo(node.current.absoluteFilePath(file)).size();
        //fi.TTH = hf.Go(node.current.absoluteFilePath(file));
        node.files.append(fi);
    }
    QStringList listDirs = node.current.entryList(QDir::Dirs);
    foreach(QString subdir, listDirs)
    {
        if(subdir == "." || subdir == "..") continue;
        DirsTree childNode;
        childNode.current = node.current;
        childNode.current.cd(subdir);
        scan(childNode);
        node.childDirs.append(childNode);
    }
}
void FileManager::SaveXML()
{
    QString xml = "xml";
    QDomDocument doc;
    QDomNode node = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"utf-8\"");
    doc.insertBefore(node, doc.firstChild());
    QDomElement docElement = doc.createElement("FileListing");
    doc.appendChild(docElement);

    QList<QDomElement> result = makeDOM(doc);
    QDomElement content;
    foreach(content, result)
        docElement.appendChild(content);



    QFile file("files.xml");

    if(file.open(QIODevice::WriteOnly))
    {
        QTextStream(&file) << doc.toString();
        file.close();
    }
}
QList<QDomElement> FileManager::makeDOM(QDomDocument& doc)
{
    DirsTree item;
    QList<QDomElement> result;

    foreach(item, tree)
    {
        QDomElement el = rootDirDOM(doc, item.current.dirName(), item.current.absolutePath());
        nodeDOM(el, doc, item);
        result.append(el);
    }
    return result;
}
void FileManager::nodeDOM(QDomElement& el, QDomDocument& doc, DirsTree & dtree)
{
    DirsTree item;
    FileInfo fi;

    foreach(item, dtree.childDirs)
    {
        QDomElement m_el = dirDOM(doc, item.current.dirName());
        nodeDOM(m_el, doc, item);
        el.appendChild(m_el);
    }
    foreach(fi, dtree.files)
    {
        QDomElement m_el = fileDOM(doc, fi);
        el.appendChild(m_el);
    }
}
QDomElement FileManager::dirDOM(QDomDocument& doc, QString str)
{
    QDomElement element = doc.createElement("Directory");
    QDomAttr atr = doc.createAttribute("Name");
    atr.setValue(str);
    element.setAttributeNode(atr);
    return element;
}
QDomElement FileManager::rootDirDOM(QDomDocument& doc, QString str, QString path)
{
    QDomElement element = doc.createElement("RootDirectory");
    QDomAttr atr = doc.createAttribute("Name");
    atr.setValue(str);
    element.setAttributeNode(atr);

    atr = doc.createAttribute("Path");
    atr.setValue(path);
    element.setAttributeNode(atr);

    return element;
}
QDomElement FileManager::fileDOM(QDomDocument& doc, FileInfo& fi)
{
    QDomElement element = doc.createElement("File");
    QDomAttr atr = doc.createAttribute("Name");
    atr.setValue(fi.filename);
    element.setAttributeNode(atr);

    atr = doc.createAttribute("TTH");
    atr.setValue(fi.TTH);
    element.setAttributeNode(atr);

    atr = doc.createAttribute("Size");
    atr.setValue(QString::number(fi.size));
    element.setAttributeNode(atr);

    return element;
}

void  FileManager::traverseNode(const QDomNode& node, DirsTree & dtree, int level)
{
    QDomNode domNode = node.firstChild();
    QString str;
    while(!domNode.isNull()) {
        if(domNode.isElement()) {
            QDomElement domElement = domNode.toElement();
            if(!domElement.isNull()) {
                str = domElement.attribute("Name");
                qDebug() << "TagName: " << domElement.tagName() << "\tName: " << str;
            }
        }
        traverseNode(domNode, dtree, 0);
        domNode = domNode.nextSibling();
    }
}

void FileManager::LoadXML()
{
    DirsTree  s;
    QFile file("files.xml");
    loadedDoc.clear();

    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    if(file.open(QIODevice::ReadOnly)) {
        if(loadedDoc.setContent(&file)) {
            QDomElement domElement = loadedDoc.documentElement();
            traverseNode(domElement, s, 0);
        }
    }
}
