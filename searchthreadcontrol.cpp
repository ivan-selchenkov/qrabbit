#include "searchthreadcontrol.h"
#include <QtSql>
#include "searchmanager.h"

SearchThreadControl::SearchThreadControl()
{
}
void SearchThreadControl::run()
{
    SearchManager sm;

    connect(this, SIGNAL(signal_income_search(SearchItem)),
            &sm, SLOT(slot_search(SearchItem)), Qt::QueuedConnection);

    connect(&sm, SIGNAL(signal_search_result(FileInfo, SearchItem)),
            this, SIGNAL(signal_outcome_search(FileInfo,SearchItem)), Qt::QueuedConnection);

    SearchThreadControl::exec();

    qDebug() << "SearchThreadControl::~run()";
}
