/*
* Copyright (c) 2016 Jake Dharmasiri.
* Licensed under the GNU GPLv3 License. See LICENSE for details.
*/

#ifndef EDITOR_STACK_H
#define EDITOR_STACK_H

#include "code_editor_interface.h"
#include "src/python/qpyconsole.h"
#include <qtabwidget.h>

class EditorStack : public QTabWidget {
    Q_OBJECT

public:
    EditorStack(QSettings* s, QWidget *parent);
    CodeEditor* currentEditor();
    QPyConsole* pyConsole;

private:
    void fileStream(CodeEditor* c, QFile* saveFile);
    void refresh(CodeEditor* c);
    int generateUntrackedID();
    QMap<int, CodeEditor*> untrackedFiles;
    QSettings* settingsPtr;
    bool modificationQueued = false;
    bool saveQueued = false;
    int globalZoom = 12;

private Q_SLOTS:
    void manageFocus();
    void flagAsModified(bool);
    void manageExternalModification();

public Q_SLOTS:
    CodeEditor* insertEditor(const QString &filePath = "");
    void open(QFile* openFile = nullptr);
    void save(int index = -1, bool forceSave = false);
    int saveAs();
    void saveAll();
    void closeTab(int index = -1, bool forceClose = false);
    void closeAll();
    void run();
    void undo();
    void redo();
    void cut();
    void copy();
    void paste();
    void selectAll();
    void zoomIn();
    void zoomOut();
    void resetZoom();
};

#endif // EDITOR_STACK_H
