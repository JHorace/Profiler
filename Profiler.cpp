#include "Profiler.h"
#include <intrin.h>
#include <algorithm>
#include <iostream>
#include <Windows.h>
#include <DbgHelp.h>
#pragma intrinsic(__rdtsc)

Profiler* Profiler::instance = nullptr;
ProfilerDestroyer  Profiler::destroyer;

Profiler::Profiler()
{
  SymInitialize(GetCurrentProcess(), NULL, true);
}

Profiler::~Profiler()
{
}

Profiler * Profiler::GetInstance()
{
  if (!instance)
  {
    instance = new Profiler();
    destroyer.SetSingleton(instance);
  }

  return instance;
}

void Profiler::Enter(unsigned __int64 time, void * signature)
{
  if (!current)
  {
    current = new FunctionNode(signature, nullptr);
  }
  else if (signature == current->GetID())
  {
    current->IncrementRecursion();
    return;
  }
  else
  {
    std::vector<FunctionNode*> children = current->GetChildren();
    std::vector<FunctionNode*>::iterator it = std::find_if(children.begin(), children.end(), [signature](FunctionNode* arg){
                                                                          return arg->GetID() == signature; });
    if (it == children.end())
    {
      FunctionNode* child = new FunctionNode(signature, current);
      current->AddChild(child);
      current = child;
    }
  }

  current->SetStartTime(time);
}

void Profiler::Leave(unsigned __int64 time)
{
  if (current->GetRecursion() > 0)
  {
    current->DecrementRecursion();
    return;
  }
  else
  {
    current->AddTotalTime(time);
    if (current->GetParent())
      current = current->GetParent();
    return;
  }
}

void Profiler::Profile()
{
  file.open("profile.log");

  current->PrintNode(file, 0);

  file.close();

}

void * FunctionNode::GetID()
{
  return ID;
}

void FunctionNode::IncrementRecursion()
{
  ++recursion;
}

void FunctionNode::DecrementRecursion()
{
  --recursion;
}

void FunctionNode::AddChild(FunctionNode * child)
{
  children.push_back(child);
}

FunctionNode * FunctionNode::GetParent()
{
  return parent;
}

unsigned FunctionNode::GetRecursion()
{
  return recursion;
}

 const std::vector<FunctionNode*>& FunctionNode::GetChildren() 
{
  return children;
}

 void FunctionNode::SetStartTime(unsigned __int64 time)
 {
   start_time = time;
 }

 void FunctionNode::AddTotalTime(unsigned __int64 time)
 {
   total_time += (time - start_time);
 }

 void FunctionNode::PrintNode(std::ofstream& stream, int level)
 {
   DWORD64 dwDisplacement = 0;
   DWORD64 dwAddress = (DWORD64) ID;

   char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
   PSYMBOL_INFO pSymbol = (PSYMBOL_INFO)buffer;

   pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
   pSymbol->MaxNameLen = MAX_SYM_NAME;

   if (SymFromAddr(GetCurrentProcess(), dwAddress, &dwDisplacement, pSymbol))
   {
     int i = 1;
   }
   else
   {
     return;
   }
   for (int i = 0; i < level; ++i)
     stream << "\t";
   stream << "Function Name: " << pSymbol->Name << "\n";
   for (int i = 0; i < level; ++i)
     stream << "\t";
   stream << "Cycles in Function: " << total_time << "\n\n";
   for (auto node : children)
     node->PrintNode(stream, level + 1);
 }

 void FunctionNode::SerializeNode(Json::Value& root)
 {

   DWORD64 dwDisplacement = 0;
   DWORD64 dwAddress = (DWORD64)ID;

   char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
   PSYMBOL_INFO pSymbol = (PSYMBOL_INFO)buffer;

   pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
   pSymbol->MaxNameLen = MAX_SYM_NAME;

   if (SymFromAddr(GetCurrentProcess(), dwAddress, &dwDisplacement, pSymbol))
   {
     int i = 1;
   }
   else
   {
     return;
   }

   root["Children"] = Json::Value(Json::arrayValue);
   root["Function Name"] = pSymbol->Name;
   root["Cycles"] = total_time;


   for (int j = 0; j < children.size(); j++)
   {
     children[j]->SerializeNode(root["Children"][j]);
   }

   
 }

 extern "C" void __declspec(naked) _cdecl _penter(void)
 {
   _asm push ebp;
   _asm mov ebp, esp;
   _asm pushad;
   Profiler::GetInstance()->Enter(__rdtsc(), _ReturnAddress());
   _asm popad;
   _asm mov esp, ebp;
   _asm pop ebp;
   _asm ret;
 }

 extern "C" void __declspec(naked) _cdecl _pexit(void)
 {
   _asm push ebp;
   _asm mov ebp, esp;
   _asm pushad;
   Profiler::GetInstance()->Leave(__rdtsc());
   _asm popad;
   _asm mov esp, ebp;
   _asm pop ebp;
   _asm ret;
 }

 ProfilerDestroyer::~ProfilerDestroyer()
 {
   if (profiler)
   {
     profiler->Profile();
     delete profiler;;
   }
 }
