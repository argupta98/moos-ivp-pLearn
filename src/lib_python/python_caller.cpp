#include "python_caller.h"
#include <dlfcn.h>
using namespace std;

PythonCaller::PythonCaller(){
  //start python interpreter
  try{
    dlopen("libpython2.7.so", RTLD_LAZY | RTLD_GLOBAL);
  }
  catch(std::exception e){
    std::cout<< e.what();
  }
  Py_Initialize();
  m_outfile.open("python_output.csv");
  m_outfile<<"Started Interpreter\n";
  m_outfile.flush();
}

void PythonCaller::setProgram(string name){
  m_outfile<<"Importing module "<<name<<"...";
  m_outfile.flush();
  pModule = PyImport_ImportModule(name.c_str());
  if(!pModule) {
    m_outfile << "Module "<<name<<" not found!!!\n";
    PyObject *ptype, *pvalue, *ptraceback;
    PyErr_Fetch(&ptype, &pvalue, &ptraceback);
    //pvalue contains error message
    char *ErrorMessage = PyBytes_AsString(pvalue);
    m_outfile << "Failed to load module with error: "<<ErrorMessage<<"\n";
    printf("Error loading Module\n\n");
    PyErr_Print();
    printf("\n\n");
  }
  else{
    m_outfile<<"Done\n";
    m_outfile.flush();
  }
}

void PythonCaller::addFuncs(vector<string> funcs){
  m_outfile<<"Adding functions...";
  m_outfile.flush();
  if(pModule){
    for(int i = 0; i < funcs.size(); ++i){
      try{
	m_outfile<<"Extracting function "<< funcs[i]<<"\n" ;
	m_outfile.flush();
	pFunc = PyObject_GetAttrString(pModule, funcs[i].c_str());
	m_outfile<<"Placing function into dictionary...";
	m_outfile.flush();
	pFuncs.insert(make_pair(funcs[i], pFunc));
	m_outfile<<"done\n Decrementing reference count to pFunc";
	m_outfile.flush();
	Py_DECREF(pFunc);
	m_outfile<<"...done\n";
	m_outfile.flush();
      }
      catch(exception e){
	m_outfile<<"\n \n Exception getting function: "<<funcs[i];
	m_outfile.flush();
      }
    }
  }
  else{
    m_outfile << "Error loading functions: No Module Available\n";
    m_outfile.flush();
  }
}

PythonCaller::~PythonCaller(){
  //DECREF all variables on destruction to free up memory
  Py_DECREF(pModule);
  Py_DECREF(pFunc);
  Py_DECREF(pTemp);
  Py_DECREF(pArgs);
  Py_DECREF(pResult);
  m_outfile.close();
  //stop python interpreter
  Py_Finalize();
}

PyObject * PythonCaller::call(string method, vector<string> s_args, vector<PyObject*> p_args, vector<vector<string> > v_args){
  m_outfile<<"calling function "<<method<<"...\n";
  if(pModule != NULL){
    //get function from the module
    pFunc = pFuncs[method]; //INCREF pFunc
    printf("finished loading function");
    if(pFunc && PyCallable_Check(pFunc)){

      //create python tuple of length args
      pArgs = PyTuple_New(p_args.size()+s_args.size()+v_args.size()); //INCREF pArgs
      int elm = 0;
      int arg = 0;
      for(arg = 0; arg < p_args.size(); ++arg){
	PyTuple_SetItem(pArgs, arg+elm, p_args[arg]);
      }
      elm += arg;
      
      for(arg = 0; arg < s_args.size(); ++arg){
	PyTuple_SetItem(pArgs, arg+elm, PyUnicode_FromString(s_args[arg].c_str()));
      }
      elm += arg;
      
      for(arg = 0; arg < v_args.size(); ++arg){
	//make python list
	pTemp = PyList_New(v_args[arg].size()); //INCREF pTemp
	//populate list
	for(int i = 0; i < v_args[arg].size(); ++i){
	  PyList_SetItem(pTemp, i, PyUnicode_FromString(v_args[arg][i].c_str())); 
	}
	PyTuple_SetItem(pArgs, arg+elm, pTemp); //DECREF pTemp
	//put into pArgs
	  
      }
      printf("Calling Function... \n");
      pResult = PyObject_CallObject(pFunc, pArgs); //INCREF pResult
      
      //DECREF pArgs, since we no longer need it
      //Py_DECREF(pArgs);
      
      if(pResult == NULL){
	m_outfile<<"Call Failed\n";
	m_outfile.flush();
	PyObject *ptype, *pvalue, *ptraceback;
	PyErr_Fetch(&ptype, &pvalue, &ptraceback);
	char *ErrorMessage = PyBytes_AsString(pvalue);
	m_outfile << "Failed to to get result with error: "<<ErrorMessage<<"\n";
	m_outfile.flush();
	PyErr_Print();
	fprintf(stderr, "Call Failed\n");
	
      }
    }
    
    //handle error in loading function
    else {
      m_outfile<< "Cannot find function "<<method<<"\n";
      m_outfile.flush();
      if (PyErr_Occurred()){
	PyObject *ptype, *pvalue, *ptraceback;
	PyErr_Fetch(&ptype, &pvalue, &ptraceback);
	char *ErrorMessage = PyBytes_AsString(pvalue);
	m_outfile << "Failed to call function with error: "<<ErrorMessage<<"\n";
	m_outfile.flush();
	PyErr_Print();
      }
      
      fprintf(stderr, "Cannot find function \"%s\"\n", method.c_str());
    }
  }
  
  //handle error in importing module
  /*NOTE: if ImportError that python module file does not exist, 
    set PYTHONPATH=. in .bashrc to include current directory */
  else {
    m_outfile << "Function call failed: Program not found\n";
    printf("Error loading Module");
    PyErr_Print();
  }
  m_outfile<<"Done calling Function!\n";
  m_outfile.flush();
  
  //stop the interpreter and return the resulting PyObject*
  return(pResult);
}

