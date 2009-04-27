#include "filemanager.h"
#include "tth/hashfile.h"

FileManager::FileManager()
{
    folders.append(QDir("/home/ivan/local/"));
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
        fi.TTH = hf.Go(node.current.absoluteFilePath(file));
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
    QDomElement file1 = fileDOM(doc, "XXX.exe");
    docElement.appendChild(file1);
    QFile file("files.xml");

    if(file.open(QIODevice::WriteOnly))
    {
        QTextStream(&file) << doc.toString();
        file.close();
    }
}
QDomElement FileManager::fileDOM(QDomDocument& doc, QString str)
{
    QDomElement element = doc.createElement("File");
    QDomAttr atr = doc.createAttribute("Name");
    atr.setValue(str);
    element.setAttributeNode(atr);
    return element;
}
