#include "filemanager.h"
#include "mainwindow.h"

FileManager::FileManager(QObject* parent): QObject(parent)
{
    folders.append(QDir("/home/share/DC_SHARE"));

    ctth = new CalculateTTH(this, &tree);

    connect(ctth, SIGNAL(newInfo(QString)), (MainWindow*) parent, SLOT(on_info(QString)));
    connect(ctth, SIGNAL(saveTree(bool)), this, SLOT(slotSaveXML(bool)));

    loadXML();
    scanFiles();
    setTTH();

    ctth->start(QThread::LowPriority);

    //saveXML();
}
FileManager::~FileManager()
{
    ctth->terminate();
}
void FileManager::scanFiles()
{
    for(int i=0; i<folders.size(); i++)
    {
        DirsTree dir;
        if(!QDir(folders.at(i)).exists()) continue;
        dir.current = folders.at(i);
        scan(dir);
        tree.append(dir);
    }

    //SaveXML();
}
void FileManager::scan(DirsTree& node)
{
    FileInfo fi;

    QStringList listFiles = node.current.entryList(QDir::Files);
    foreach(QString file, listFiles)
    {
        fi.clear();
        fi.filename = file;
        fi.dir = node.current;
        fi.size = QFileInfo(node.current.absoluteFilePath(file)).size();
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
void FileManager::slotSaveXML(bool startAnywhere)
{
    if(startAnywhere) { saveXML(); return; }

    if(time.elapsed() == 0) // first start
    {
        saveXML();
        time.start();
    }
    else if(time.elapsed() > 30000)
    {
        time.restart();
        saveXML();
    }
}
void FileManager::saveXML()
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
        if(!fi.TTH.isEmpty())
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

void  FileManager::traverseNode(const QDomNode& node, DirsTree & dtree)
{
    QDomNode domNode = node.firstChild();
    QString str;
    FileInfo _file;
    DirsTree _newDirTree;

    while(!domNode.isNull()) { // parsing current node
        if(domNode.isElement()) {
            QDomElement domElement = domNode.toElement();
            if(!domElement.isNull()) {
                if(domElement.tagName() == "Directory") // if this is directiry
                {
                    _newDirTree.clear();
                    _newDirTree.current = dtree.current; // setting current dir
                    _newDirTree.current.cd(domElement.attribute("Name")); // down to new dir
                    traverseNode(domNode, _newDirTree);
                    dtree.childDirs.append(_newDirTree);
                }
                else if(domElement.tagName() == "File")
                {
                    bool ok;
                    _file.clear();
                    _file.dir = dtree.current;
                    _file.filename = domElement.attribute("Name");
                    _file.size = domElement.attribute("Size").toULongLong(&ok);
                    _file.TTH = domElement.attribute("TTH");

                    if(ok)
                        dtree.files.append(_file);
                }
                qDebug() << "TagName: " << domElement.tagName() << "\tName: " << str;
            }
        }
        //traverseNode(domNode, dtree);
        domNode = domNode.nextSibling();
    }
}

void FileManager::loadXML()
{
    DirsTree  s;
    QFile file("files.xml");
    loadedDoc.clear();

    // Setting up encoding to UTF8
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

    if(file.open(QIODevice::ReadOnly)) { // file is successfully opened
        if(loadedDoc.setContent(&file)) { // XML content is parsed
            QDomElement domElement = loadedDoc.documentElement(); // Main element
            rootDirParse(domElement);
        }
    }
}
void FileManager::rootDirParse(const QDomNode& node)
{
    QDomNode domNode = node.firstChild();
    DirsTree _newDirTree;

    while(!domNode.isNull()) { // parsing RootDirectory only
        if(domNode.isElement()) {
            QDomElement domElement = domNode.toElement();
            if(!domElement.isNull()) {
                if(domElement.tagName() == "RootDirectory") {                    
                    qDebug() << "Name: " << domElement.attribute("Path");
                    _newDirTree.clear();
                    _newDirTree.current = QDir(domElement.attribute("Path"));
                    traverseNode(domNode, _newDirTree);
                    loadedTree.append(_newDirTree);
                }
            }
        }
        domNode = domNode.nextSibling();
    }
}

void FileManager::setTTH()
{
    DirsTree loadedRootTree;
    DirsTree realRootTree;
    int index;
    int i;
    // going throw loadedTree and setting TTH in tree
    foreach(loadedRootTree, loadedTree)
    {
        // finding roots  in tree
        index = tree.indexOf(loadedRootTree);
        if(index >= 0) {
            setTTHDirectory(loadedRootTree, tree[index]);
        }
    }
}
void FileManager::setTTHDirectory(DirsTree &loadedTree, DirsTree &realTree)
{
    DirsTree loadedDir, realDir;
    FileInfo loadedFi, realFi;
    FileInfo current;
    QFileInfo fileinfo;
    int index;
    HashFile hf;

    foreach(loadedDir, loadedTree.childDirs) // comparing childDirs
    {
        index = realTree.childDirs.indexOf(loadedDir);
        if(index >= 0) {
            setTTHDirectory(loadedDir, realTree.childDirs[index]);
        }
    }

    foreach(loadedFi, loadedTree.files)
    {
        index = realTree.files.indexOf(loadedFi);
        if(index >= 0)
                realTree.files[index].TTH = loadedFi.TTH;
    }
}
