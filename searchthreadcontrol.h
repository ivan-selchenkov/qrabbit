#ifndef SEARCHTHREADCONTROL_H
#define SEARCHTHREADCONTROL_H

#include "searchitem.h"
#include "fileinfo.h"
#include <QThread>

class SearchThreadControl : public QThread
{
    Q_OBJECT
public:
    SearchThreadControl();
protected:
    void run();
signals:
    void signal_income_search(SearchItem si);
    void signal_outcome_search(FileInfo itemslist, SearchItem si);
};

#endif // SEARCHTHREADCONTROL_H
