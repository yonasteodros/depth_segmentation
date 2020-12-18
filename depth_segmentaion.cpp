/*
 * This file is part of ElasticFusion.
 *
 * Copyright (C) 2015 Imperial College London
 * 
 * The use of the code within this file and all code within files that 
 * make up the software that is ElasticFusion is permitted for 
 * non-commercial purposes only.  The full terms and conditions that 
 * apply to the code within this file are detailed within the LICENSE.txt 
 * file and at <http://www.imperial.ac.uk/dyson-robotics-lab/downloads/elastic-fusion/elastic-fusion-license/> 
 * unless explicitly stated.  By downloading this file you agree to 
 * comply with these terms.
 *
 * If you wish to use any of this code for commercial purposes then 
 * please email researchcontracts.engineering@imperial.ac.uk.
 *
 */

#include "MainController.h"
#include "python3.7m/Python.h"
#include <iostream>
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include "numpy/ndarraytypes.h"
#include "numpy/arrayobject.h"
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv/cv.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <thread>  
using namespace cv;


using namespace std;
#define DO_NOT_RUN_ANY_PYTHON_IN_MAIN_THREAD

PyObject *pModule = NULL;
PyObject *pExecute  = NULL;
PyObject *result  = NULL;
PyObject *m_PyDict  = NULL;
PyObject *m_Pyfunc  = NULL;
PyObject* po_result  = NULL;


void main_thread_GIL_not_locked(cv::Mat ImageData);

void reference_counting() {
    Py_XDECREF(pModule);
    Py_DECREF(m_PyDict);
    Py_DECREF(m_Pyfunc);
    //Py_XDECREF(po_result);
    Py_XDECREF(pExecute);
    Py_Finalize();
}

PyObject *convertImage(const cv::Mat image){

     npy_intp dims[3] = { image.rows, image.cols, 3 };
     return PyArray_SimpleNewFromData(3,dims, NPY_UINT8, image.data);

}

void  imageimport(cv::Mat ImageData)
{


      Py_XDECREF(PyObject_CallFunctionObjArgs(pExecute, convertImage(ImageData), NULL));
      std::cout << "imageimport is ended" << std::endl;

}



void initialise(){


    //wchar_t *program = Py_DecodeLocale(argv[0], NULL);
    setenv("PYTHONPATH", ".", 1);
  // Python initialize
   Py_SetProgramName((wchar_t*)L"calc");

    //PyRun_SimpleString("import sys\n" "print(sys.path)\n");
  wchar_t const * argv2[] = { L"/media/user/Data/DeepLearning/Fusion/VicariosFusion/src/calc.py" };
  PySys_SetArgv(1, const_cast<wchar_t**>(argv2));
  PyRun_SimpleString("import sys\n""print(sys.path)\n");

   std::cout << " * Loading module..." << std::endl;
   PyObject *pModule = PyImport_ImportModule("calc");
   if(pModule == NULL) {
       if(PyErr_Occurred()) {
           std::cout << "Python error indicator is set:" << std::endl;
           PyErr_Print();
       }
       throw std::runtime_error("Could not open calc module.");
   }


   // get dictionary of available items in the module
    PyObject *m_PyDict = PyModule_GetDict(pModule);

    // grab the functions we are interested in
    PyObject *m_Pyfunc = PyDict_GetItemString(m_PyDict, "execute");

    // execute the function
    if (m_Pyfunc != nullptr)
    {

      // Get function
      pExecute = PyObject_GetAttrString(pModule, "execute");
      if(pExecute == NULL || !PyCallable_Check(pExecute)) {
          if(PyErr_Occurred()) {
              std::cout << "Python error indicator is set:" << std::endl;
              PyErr_Print();
          }
          throw std::runtime_error("Could not load function 'execute' from calc module.");
      }
      std::cout << "* Initialised calc" << std::endl;

      //result = PyEval_CallObject( pExecute, NULL );
      //Py_XDECREF(PyObject_CallFunctionObjArgs(pExecute, convertImage(Input), NULL));

      cout<<"result is working"<<endl;
       }
}

void bar(int x)
{
  std::cout << "main, foo and bar now execute concurrently...\n";
}

int main(int argc, char *argv[])
{
    
    
    
    cv::Mat img = cv::imread("Tabel_3.jpg", cv::IMREAD_COLOR);

      if(img.empty())
    {
        std::cout << "Could not read the image: " << std::endl;
        return 1;
    }
  
   //imshow("Display window", img);
   //int k = waitKey(0); // Wait for a keystroke in the window

   Py_Initialize();
   PyEval_InitThreads();
   initialise();
   import_array();

   auto threadState = PyEval_SaveThread();
   main_thread_GIL_not_locked(img);

      //MainController mainController(argc, argv);
   //mainController.launch();
   
  // std::thread first (imageimport,img);     // spawn new thread that calls foo()
  // std::thread second (bar,0);  // spawn new thread that calls bar(0)


   
   //first.join();                // pauses until first finishes
   //second.join();               // pauses until second finishes

   //std::cout << "foo and bar completed.\n";
   PyEval_RestoreThread(threadState);
   //Py_XDECREF(pModule);
   //Py_DECREF(m_PyDict);
  // Py_DECREF(m_Pyfunc);
   //Py_XDECREF(po_result);
   //Py_XDECREF(pExecute);
   Py_Finalize();

    return 0;
}

// Grab the object and print the redirection.
void print_intercepted_stdout()
{
    auto state = PyGILState_Ensure();

    PyObject *sysmodule;
    PyObject *pystdout;
    PyObject *pystdoutdata;
    char *stdoutstring;
    sysmodule = PyImport_ImportModule("sys");
    pystdout = PyObject_GetAttrString(sysmodule, "stdout");
    pystdoutdata = PyObject_GetAttrString(pystdout, "data");
    //stdoutstring = PyString_AsString(pystdoutdata);

    //std::cout << stdoutstring;

    PyGILState_Release(state);
    std::cout << "Python output printed" << std::endl;
}
void worker_thread_GIL_not_locked(cv::Mat ImageData)
{
    std::stringstream sstr;
    sstr << "print('Worker Thread ')";
    auto state = PyGILState_Ensure();
    imageimport(ImageData);
    auto result = PyRun_SimpleString(sstr.str().c_str());
    assert(0 == result);
    PyGILState_Release(state);
}
// Redirect Python prints to an object so we can
// print them in the console.
void intercept_python_stdout()
{
    std::cout << "Intercepting Python output" << std::endl;
    auto state = PyGILState_Ensure();

    /*auto result = PyRun_SimpleString("\
class StdoutCatcher:\n\
    def __init__(self):\n\
        self.data = ''\n\
    def write(self, stuff):\n\
        self.data = self.data + stuff\n\
import sys\n\
sys.stdout = StdoutCatcher()");
    assert(0 == result);*/

    PyGILState_Release(state);
}

void main_thread_GIL_not_locked(cv::Mat ImageData)
{

    //std::thread(intercept_python_stdout).join();

    // Launch a bunch of worker threads that will lock and
    // do Python things.
    const auto t_count = 2;
    std::thread t[t_count];
    for (auto i=0; i<t_count; ++i)
    {
        t[i] = std::thread(worker_thread_GIL_not_locked, ImageData);
    }
    // Wait for them to complete.
    for (auto i=0; i<t_count; ++i)
    {
        t[i].join();
    }

    //std::thread(print_intercepted_stdout).join();

}