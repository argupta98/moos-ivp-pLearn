
#ifndef PYTHON_CALLER
#define PYTHON_CALLER

#include <Python.h>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <fstream>

class PythonCaller {
 public:
  PythonCaller();
  ~PythonCaller();
  void setProgram(std::string);
  void addFuncs(std::vector<std::string>);
  PyObject * call(std::string, std::vector<std::string>, std::vector<PyObject*>, std::vector<std::vector<std::string> >);
 private:
  PyObject *pFunc, *pModule, *pArgs, *pTemp, *pResult;
  std::unordered_map<std::string, PyObject*> pFuncs;
  std::ofstream m_outfile;
};
 
#endif
