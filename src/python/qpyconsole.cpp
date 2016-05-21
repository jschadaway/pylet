/*
  QPyConsole.cpp

  Controls the GEMBIRD Silver Shield PM USB outlet device

  (C) 2006, Mondrian Nuessle, Computer Architecture Group, University of Mannheim, Germany

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


  nuessle@uni-mannheim.de

*/

// modified by YoungTaek Oh.
// modified by Jake Dharmasiri.

//#ifdef WIN32
//#   undef _DEBUG
//#endif
#include "qpyconsole.h"

#include <QDebug>
#include <fstream>

PyObject* glb;
PyObject* loc;

QString resultString;

static PyObject* redirector_init(PyObject *, PyObject *)
{
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject* redirector_write(PyObject *, PyObject *args)
{
    char* output;
    PyObject *selfi;

    if (!PyArg_ParseTuple(args,"s",&output))
    {
        return NULL;
    }
    QString outputString = QString::fromLocal8Bit(output);
    resultString.append(outputString);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject* redirector_flush(PyObject *, PyObject *args) {
    resultString = "";
    Py_INCREF(Py_None);
    return Py_None;
}

static PyMethodDef redirectorMethods[] =
{
    {"__init__", redirector_init, METH_VARARGS,
     "initialize the stdout/err redirector"},
    {"write", redirector_write, METH_VARARGS,
     "implement the write method to redirect stdout/err"},
     { "flush", redirector_flush, METH_VARARGS,
     "implement the flush method to redirect stdout/err" },
    {NULL,NULL,0,NULL},
};

static PyObject* py_clear(PyObject *, PyObject *)
{
    QPyConsole::getInstance()->clear();
    QFont monoFont = QFont("Courier New", 12, QFont::Normal, false);
    QPyConsole::getInstance()->setFont(monoFont);
    return Py_None;
}

static PyObject* py_reset(PyObject *, PyObject *)
{
    QPyConsole::getInstance()->reset();
    return Py_None;
}

static PyObject* py_save(PyObject *, PyObject *args)
{
    char* filename;
    if (!PyArg_ParseTuple(args,"s",&filename))
    {
        return NULL;
    }
    QPyConsole::getInstance()->saveScript(filename);
    return Py_None;
}

static PyObject* py_load(PyObject *, PyObject *args)
{
    char* filename;
    if (!PyArg_ParseTuple(args,"s",&filename))
    {
        return NULL;
    }
    QPyConsole::getInstance()->loadScript(filename);

    return Py_None;
}

static PyObject* py_history(PyObject *, PyObject *)
{
    QPyConsole::getInstance()->printHistory();
    return Py_None;
}

static PyObject* py_quit(PyObject *, PyObject *)
{
    resultString="Use reset() to restart the interpreter; otherwise exit your application\n";
    return Py_None;
}

static PyMethodDef ModuleMethods[] = { {NULL,NULL,0,NULL} };
static PyMethodDef console_methods[] =  {
    {"clear",py_clear, METH_VARARGS,"clears the console"},
    {"reset",py_reset, METH_VARARGS,"reset the interpreter and clear the console"},
    {"save",py_save, METH_VARARGS,"save commands up to now in given file"},
    {"load",py_load, METH_VARARGS,"load commands from given file"},
    {"history",py_history, METH_VARARGS,"shows the history"},
    {"quit",py_quit, METH_VARARGS,"print information about quitting"},

    {NULL, NULL,0,NULL}
};

typedef struct {
    PyObject_HEAD
} redirector_redirectorObject;

static PyTypeObject redirector_redirectorType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "redirector.redirector",             /* tp_name */
    sizeof(redirector_redirectorObject), /* tp_basicsize */
    0,                                   /* tp_itemsize */
    0,                                   /* tp_dealloc */
    0,                                   /* tp_print */
    0,                                   /* tp_getattr */
    0,                                   /* tp_setattr */
    0,                                   /* tp_reserved */
    0,                                   /* tp_repr */
    0,                                   /* tp_as_number */
    0,                                   /* tp_as_sequence */
    0,                                   /* tp_as_mapping */
    0,                                   /* tp_hash  */
    0,                                   /* tp_call */
    0,                                   /* tp_str */
    0,                                   /* tp_getattro */
    0,                                   /* tp_setattro */
    0,                                   /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,                  /* tp_flags */
    "",                                  /* tp_doc */
    0,                                   /* tp_traverse */
    0,                                   /* tp_clear */
    0,                                   /* tp_richcompare */
    0,                                   /* tp_weaklistoffset */
    0,                                   /* tp_iter */
    0,                                   /* tp_iternext */
    redirectorMethods                    /* tp_methods */
};

static struct PyModuleDef redirector =
{
    PyModuleDef_HEAD_INIT,
    "redirector",
    "",
    -1,
    ModuleMethods
};

static struct PyModuleDef console =
{
    PyModuleDef_HEAD_INIT,
    "console",
    "",
    -1,
    console_methods
};

PyMODINIT_FUNC PyInit_redirector(void) 
{
    PyObject* redirectModule;

    redirector_redirectorType.tp_new = PyType_GenericNew;
    PyType_Ready(&redirector_redirectorType);

    redirectModule = PyModule_Create(&redirector);

    Py_INCREF(&redirector_redirectorType);
    PyModule_AddObject(redirectModule, "redirector", (PyObject *) &redirector_redirectorType);
    return redirectModule;
}

PyMODINIT_FUNC PyInit_console(void) 
{
    return PyModule_Create(&console);
}

void initredirector()
{
    PyMethodDef *def;

    /* create a new module and class */
    PyObject *module = PyInit_redirector();
    PyObject *moduleDict = PyModule_GetDict(module);
    PyObject *classDict = PyDict_New();
    PyObject *classBases = PyTuple_New(0);
    PyObject *className = PyUnicode_FromString("redirector");
    PyObject *fooType = PyType_GenericNew(&PyType_Type, classDict, className);
    PyDict_SetItemString(moduleDict, "redirector", fooType);
    Py_DECREF(classDict);
    Py_DECREF(classBases);
    Py_DECREF(className);
    Py_DECREF(fooType);

    /* add methods to class */
    for (def = redirectorMethods; def->ml_name != NULL; def++) {
        PyObject *func = PyCFunction_New(def, NULL);
        PyObject *method = PyInstanceMethod_New(func);
        PyDict_SetItemString(classDict, def->ml_name, method);
        Py_DECREF(func);
        Py_DECREF(method);
    }
}

void QPyConsole::printHistory()
{
    uint index = 1;
    for ( QStringList::Iterator it = history.begin(); it != history.end(); ++it )
    {
        resultString.append(QString("%1\t%2\n").arg(index).arg(*it));
        index ++;
    }
}

QPyConsole *QPyConsole::theInstance = NULL;

QPyConsole *QPyConsole::getInstance(QWidget *parent, const QString& welcomeText)
{
    if (!theInstance)
    {
        theInstance = new QPyConsole(parent, welcomeText);
    }
    return theInstance;
}

void QPyConsole::launchPythonInstance(bool firstRun) {
    if (firstRun) 
    {
        //set the Python Prompt
        setNormalPrompt(true);
    }

    Py_Finalize();

    // inject wrapper modules
    PyImport_AppendInittab("redirector", &PyInit_redirector);
    PyImport_AppendInittab("console", &PyInit_console);

    Py_Initialize();
    /* NOTE: In previous implementaion, local name and global name
    were allocated separately.  And it causes a problem that
    a function declared in this console cannot be called.  By
    unifying global and local name with __main__.__dict__, we
    can get more natural python console.
    */
    PyObject *module = PyImport_ImportModule("__main__");
    loc = glb = PyModule_GetDict(module);

    PyImport_ImportModule("rlcompleter");
    PyRun_SimpleString("import sys\n"
        "import redirector\n"
        "import console\n"
        "import rlcompleter\n"
        "sys.path.insert(0, \".\")\n" // add current
        // path
        "sys.stdout = redirector.redirector()\n"
        "sys.stderr = sys.stdout\n"
        "import builtins\n"
        "builtins.clear=console.clear\n"
        "builtins.reset=console.reset\n"
        "builtins.save=console.save\n"
        "builtins.load=console.load\n"
        "builtins.history=console.history\n"
        "builtins.quit=console.quit\n"
        "builtins.completer=rlcompleter.Completer()\n"
        );
}

//QTcl console constructor (init the QTextEdit & the attributes)
QPyConsole::QPyConsole(QWidget *parent, const QString& welcomeText) :
        QConsole(parent, welcomeText),lines(0)
{
    launchPythonInstance(true);
    setWordWrapMode(QTextOption::WrapAnywhere);
}

QString QPyConsole::generateRestartString() {
    int fontWidth = fontMetrics().width('=');
    QString spacer(((width() - 100) / fontWidth) / 3, '=');
    QString restartString = "\n" + spacer + " RESTART " + spacer + "\n";
    return restartString;
}

void QPyConsole::runFile(const std::string &filename) 
{
    launchPythonInstance(false);
    append(generateRestartString());

    this->lines = 0;
    this->command = "";
    QString execString = "with open(\"" + QString::fromUtf8(filename.c_str()) + "\") as f:\n\t"
        "code = compile(f.read(), \"" + QString::fromUtf8(filename.c_str()) + "\", 'exec')\n\t"
        "exec(code)";
    execCommand(execString, false);
}

char save_error_type[1024], save_error_info[1024];

bool
QPyConsole::py_check_for_unexpected_eof()
{
    PyObject *errobj, *errdata, *errtraceback, *pystring;

    /* get latest python exception info */
    PyErr_Fetch(&errobj, &errdata, &errtraceback);

    pystring = NULL;
    if (errobj != NULL &&
        (pystring = PyObject_Str(errobj)) != NULL &&     /* str(object) */
        (PyUnicode_Check(pystring))
        )
    {
        strcpy(save_error_type, PyUnicode_AsUTF8(pystring));
    }
    else
        strcpy(save_error_type, "<unknown exception type>");
    Py_XDECREF(pystring);

    pystring = NULL;
    if (errdata != NULL &&
        (pystring = PyObject_Str(errdata)) != NULL &&
        (PyUnicode_Check(pystring))
        )
        strcpy(save_error_info, PyUnicode_AsUTF8(pystring));
    else
        strcpy(save_error_info, "<unknown exception data>");
    Py_XDECREF(pystring);

    if (strstr(save_error_type, "exceptions.SyntaxError")!=NULL &&
        strncmp(save_error_info,"('unexpected EOF while parsing',",32)==0)
    {
        return true;
    }
    PyErr_Print ();
    resultString="Error: ";
    resultString.append(save_error_info);
    Py_XDECREF(errobj);
    Py_XDECREF(errdata);         /* caller owns all 3 */
    Py_XDECREF(errtraceback);    /* already NULL'd out */
    return false;
}

//Desctructor
QPyConsole::~QPyConsole()
{
    Py_Finalize();
}

//Call the Python interpreter to execute the command
//retrieve back results using the python internal stdout/err redirectory (see above)
QString QPyConsole::interpretCommand(const QString &command, int *res)
{
    PyObject* py_result;
    PyObject* dum;
    bool multiline=false;
    *res = 0;
    if (!command.startsWith('#') && (!command.isEmpty() || (command.isEmpty() && lines!=0)))
    {
        this->command.append(command);
        py_result=Py_CompileString(this->command.toLocal8Bit().data(),"<stdin>",Py_single_input);
        if (py_result==0)
        {
            multiline=py_check_for_unexpected_eof();
            if (!multiline) {
                if (command.endsWith(':'))
                    multiline = true;
            }

            if (multiline)
            {
                setMultilinePrompt(false);
                this->command.append("\n");
                lines++;
                resultString="";
                QConsole::interpretCommand(command, res);
                return "";
            }
            else
            {
                setNormalPrompt(false);
                *res=-1;
                QString result=resultString;
                resultString="";
                QConsole::interpretCommand(command, res);
                this->command="";
                this->lines=0;
                return result;
            }
        }
        if ( (lines!=0 && command=="") || (this->command!="" && lines==0))
        {
            setNormalPrompt(false);
            this->command="";
            this->lines=0;

            dum = PyEval_EvalCode (py_result, glb, loc);
            Py_XDECREF (dum);
            Py_XDECREF (py_result);
            if (PyErr_Occurred ())
            {
                *res=-1;
                PyErr_Print ();
            }
            QString result=resultString;
            resultString="";
            if (command!="")
                QConsole::interpretCommand(command, res);
            return result;
        }
        else if (lines!=0 && command!="") //following multiliner line
        {
            this->command.append("\n");
            *res=0;
            QConsole::interpretCommand(command, res);
            return "";
        }
        else
        {
            return "";
        }

    }
    else
        return "";
}

QStringList QPyConsole::suggestCommand(const QString &cmd, QString& prefix)
{
    char run[255];
    int n =0;
    QStringList list;
    prefix = "";
    resultString="";
    if (!cmd.isEmpty()) {
        do {
            snprintf(run,255,"print completer.complete(\"%s\",%d)\n",
                     cmd.toLatin1().data(),n);
            PyRun_SimpleString(run);
            resultString=resultString.trimmed(); //strip trailing newline
            if (resultString!="None")
            {
                list.append(resultString);
                resultString="";
            }
            else
            {
                resultString="";
                break;
            }
            n++;
        } while (true);
    }
    list.removeDuplicates();
    return list;
}
