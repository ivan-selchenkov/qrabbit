#include "initfiletree.h"

InitFileTree::InitFileTree(QObject* parent, QList<DirsTree> & _t, QList<QDir>& _f): QThread(parent), tree(_t), folders(_f)
{
    qDebug() << "InitFileTree()";
}
InitFileTree::~InitFileTree()
{
    qDebug() << "~InitFileTree()";
}
void InitFileTree::run()
{
    qDebug() << "InitFileTree->run()";
    loadXML();
    scanFiles();
    setTTH();

    calcTTH();

    slotSaveXML(true);
    qDebug() << "exiting InitFileTree->run()";
    emit signal_finished();
}
void InitFileTree::calcTTH()
{
    int index;
    hashedCount = 0;

    for(index=0; index<tree.size(); index++)
        calcDirectory(tree[index]);
}
void InitFileTree::calcDirectory(DirsTree & realTree)
{
    HashFile hf;
    FileInfo file;
    int index;

    for(index=0; index<realTree.childDirs.size(); index++)
        calcDirectory(realTree.childDirs[index]);

    for(index=0; index<realTree.files.size(); index++)
    {
        hashedCount += realTree.files[index].size;

        if(realTree.files[index].TTH.isEmpty())
        {
            //emit newInfo(QFileInfo(realTree.files[index].dir, realTree.files[index].filename).absoluteFilePath());
            realTree.files[index].TTH = hf.Go(QFileInfo(realTree.files[index].dir, realTree.files[index].filename).absoluteFilePath());
            if(totalCount > 0)
                emit signal_hashing_progress(100 * hashedCount / totalCount);
        }
        slotSaveXML(false);
    }
}

void InitFileTree::loadXML()
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
void InitFileTree::rootDirParse(const QDomNode& node)
{
    QDomNode domNode = node.firstChild();
    DirsTree _newDirTree;

    while(!domNode.isNull()) { // parsing RootDirectory only
        if(domNode.isElement()) {
            QDomElement domElement = domNode.toElement();
            if(!domElement.isNull()) {
                if(domElement.tagName() == "RootDirectory") {
                    //qDebug() << "Name: " << domElement.attribute("Path");
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
void  InitFileTree::traverseNode(const QDomNode& node, DirsTree & dtree)
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
                //qDebug() << "TagName: " << domElement.tagName() << "\tName: " << str;
            }
        }
        //traverseNode(domNode, dtree);
        domNode = domNode.nextSibling();
    }
}
void InitFileTree::scanFiles()
{
    totalCount = 0;
    QDir top;

    for(int i=0; i<folders.size(); i++)
    {
        DirsTree dir;
        if(!QDir(folders.at(i)).exists()) continue;
        dir.current = folders.at(i);
        top = dir.current;
        top.cdUp();
        scan(dir, top);
        tree.append(dir);
    }

    //SaveXML();
}
void InitFileTree::scan(DirsTree& node, const QDir& top)
{
    FileInfo fi;

    QStringList listFiles = node.current.entryList(QDir::Files);
    foreach(QString file, listFiles)
    {
        fi.clear();
        fi.filename = file;
        fi.dir = node.current;
        fi.relativeDir = top.relativeFilePath(node.current.absoluteFilePath(file));
        fi.size = QFileInfo(node.current.absoluteFilePath(file)).size();
        totalCount += fi.size;
        node.files.append(fi);
    }
    QStringList listDirs = node.current.entryList(QDir::Dirs);
    foreach(QString subdir, listDirs)
    {
        if(subdir == "." || subdir == "..") continue;
        DirsTree childNode;
        childNode.current = node.current;
        childNode.current.cd(subdir);
        scan(childNode, top);
        node.childDirs.append(childNode);
    }
}
void InitFileTree::setTTH()
{
    DirsTree loadedRootTree;
    DirsTree realRootTree;
    int index;
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
void InitFileTree::setTTHDirectory(DirsTree &loadedTree, DirsTree &realTree)
{
    DirsTree loadedDir, realDir;
    FileInfo loadedFi, realFi;
    FileInfo current;
    QFileInfo fileinfo;
    int index;

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
void InitFileTree::slotSaveXML(bool startAnywhere)
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
void InitFileTree::saveXML()
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
QList<QDomElement> InitFileTree::makeDOM(QDomDocument& doc)
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
void InitFileTree::nodeDOM(QDomElement& el, QDomDocument& doc, DirsTree & dtree)
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
QDomElement InitFileTree::dirDOM(QDomDocument& doc, QString str)
{
    QDomElement element = doc.createElement("Directory");
    QDomAttr atr = doc.createAttribute("Name");
    atr.setValue(str);
    element.setAttributeNode(atr);
    return element;
}
QDomElement InitFileTree::rootDirDOM(QDomDocument& doc, QString str, QString path)
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
QDomElement InitFileTree::fileDOM(QDomDocument& doc, FileInfo& fi)
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
