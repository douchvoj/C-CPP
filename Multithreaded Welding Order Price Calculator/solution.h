#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <climits>
#include <cfloat>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <numeric>
#include <vector>
#include <set>
#include <unordered_map>
#include <queue>
#include <memory>
#include <functional>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include "progtest_solver.h"
#include "sample_tester.h"
using namespace std;

class CWeldingCompany
{
  public:
    static double getPrice(vector<CProd> m_List, COrder& order);
    static void seqSolve ( APriceList priceList, COrder& order);
    void addProducer  ( AProducer prod );
    void addCustomer  ( ACustomer cust );
    void addPriceList ( AProducer prod, APriceList priceList );
    void start        ( unsigned thrCount );
    void stop();

  private:
  
  struct WorkItem {
    AOrderList order;
    ACustomer customer;
    unsigned idOrder;
  };
  
  class MaterialInfo
  {
  public:

  MaterialInfo(unsigned id, const vector<AProducer>& producers, CWeldingCompany& parent)
    : i_Parent(parent), i_ID(id) 
{
    for (const auto& p : producers)
        i_Producers.insert(p);
};

    ~MaterialInfo(){}
    bool isReady()const;
    void addPriceListInfo(const AProducer & producer, const APriceList& priceList);
    void addToBacklog(const WorkItem& order);
    void makeUniPriceList();
    vector<WorkItem>pushFromBacklog();
    void askPriceList();

    APriceList getUniPriceList()const;
  
    private:
      CWeldingCompany& i_Parent;  
      unsigned i_ID;
      set<AProducer>i_Producers;
      set<AProducer>i_RemainingProducers; 
      
      vector<pair<bool,CProd>>i_Products;    
      vector<WorkItem>i_backlog; 
      bool i_Ready = false;
      
      APriceList UniPriceList = make_shared<CPriceList>(i_ID);
  };
  
    vector<thread> m_CustomerThreads;
    vector<thread> m_WorkerThreads;

    vector<AProducer> m_Producers;
    vector<ACustomer> m_Customers;
    unordered_map <unsigned,shared_ptr<MaterialInfo>> m_MaterialCache;
    
    queue<WorkItem> m_CustomerQue;
    queue<unsigned> m_MaterialQue;
    
    mutex m_MutexMain;
    mutex m_MutexCustomerQue;
    mutex m_MutexMaterialQue;
    mutex m_MutexStop;
    mutex m_MutexCache;

    condition_variable m_CondVar;
    condition_variable m_StopCondVar;
    condition_variable m_FinishCondVar;
    
    atomic<int> m_OrderCount{0};
    atomic<int> m_CustomersCount{0};

    atomic<int> m_ActiveWorkers{0};
    bool m_Stop = false;

    void customerThread(ACustomer customer);
    void workerThread();
    void pushToMaterialQue(const unsigned id);
    void pushToCustomerQue(const vector<WorkItem>&orders);
    void pushToWaitingQue(const unsigned id);
  };  