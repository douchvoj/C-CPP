#include "solution.h"

// Calculates the minimum cost to produce a requested sheet using available materials and welding.
// Uses dynamic programming to find the optimal combination of cuts and joins.
double CWeldingCompany::getPrice(vector<CProd> m_List, COrder& order){
    vector<vector<double>> dp(order.m_W + 1, vector<double>(order.m_H + 1, DBL_MAX));
    for (const auto& p : m_List) {
        if (p.m_W < order.m_W + 1 && p.m_H < order.m_H+1 ) {
            dp[p.m_W][p.m_H] = min(dp[p.m_W][p.m_H], p.m_Cost);
        }
        if (p.m_H < order.m_W+1 && p.m_W < order.m_H+1 ) {
            dp[p.m_H][p.m_W] = min(dp[p.m_H][p.m_W], p.m_Cost);
        }
    }
    for (size_t i = 1; i < order.m_W + 1; i++) {  
        for (size_t j = 1; j < order.m_H + 1; j++) {
            for (size_t x = 1; x < i; x++) {
                dp[i][j] = min(dp[i][j], dp[x][j] + dp[i - x][j] + j * order.m_WeldingStrength);
            }
            for (size_t y = 1; y < j; y++) {
                dp[i][j] = min(dp[i][j], dp[i][y] + dp[i][j - y] + i * order.m_WeldingStrength);
            }
            for(size_t k = 0;k<m_List.size();k++){
                if(i == m_List[k].m_W && j + m_List[k].m_H < order.m_H + 1){
                    dp[i][j + m_List[k].m_H] = min(dp[i][j + m_List[k].m_H], dp[i][j] + m_List[k].m_Cost + (order.m_WeldingStrength*i));
                }
                if(i == m_List[k].m_H && j + m_List[k].m_W < order.m_H + 1){
                    dp[i][j + m_List[k].m_W] = min(dp[i][j + m_List[k].m_W], dp[i][j] + m_List[k].m_Cost + (order.m_WeldingStrength*i));
                }
                if(j == m_List[k].m_H && i + m_List[k].m_W < order.m_W + 1){
                    dp[i + m_List[k].m_W][j] = min(dp[i + m_List[k].m_W][j], dp[i][j] + m_List[k].m_Cost + (order.m_WeldingStrength*j));
                }
                if(j == m_List[k].m_W && i + m_List[k].m_H < order.m_W + 1){
                    dp[i + m_List[k].m_H][j] = min(dp[i + m_List[k].m_H][j], dp[i][j] + m_List[k].m_Cost + (order.m_WeldingStrength*j));
                }
            }
        }
    }
    return dp[order.m_W][order.m_H];
  }
  
void CWeldingCompany::seqSolve ( APriceList priceList, COrder& order ) {
    order.m_Cost = getPrice(priceList->m_List, order);
  }

// Add a producer to the welding company
void CWeldingCompany::addProducer(AProducer prod) {
    m_Producers.push_back(prod);
}

// Add a customer to the welding company
void CWeldingCompany::addCustomer(ACustomer cust) {
    m_Customers.push_back(cust);
}

// Add a price list received from a producer to the material cache
void CWeldingCompany::addPriceList(AProducer prod, APriceList priceList) {
    {
        unique_lock<mutex> lockCache(m_MutexCache);
        m_MaterialCache[priceList->m_MaterialID]->addPriceListInfo(prod, priceList);
    }
}

// Start customer and worker threads
void CWeldingCompany::start(unsigned thrCount) {
    m_Stop = false;

    // Start a thread for each customer
    for (auto& customer : m_Customers) {
        m_CustomerThreads.emplace_back(&CWeldingCompany::customerThread, this, customer);
    }

    // Start a number of worker threads
    for (unsigned i = 0; i < thrCount; i++) {
        m_WorkerThreads.emplace_back(&CWeldingCompany::workerThread, this);
    }
}

// Push a material ID into the material queue
void CWeldingCompany::pushToMaterialQue(const unsigned id) {
    {
        lock_guard<mutex> lock(m_MutexMaterialQue);
        m_MaterialQue.push(id);
    }
}

// Push work items into the customer queue
void CWeldingCompany::pushToCustomerQue(const vector<WorkItem>& orders) {
    {
        lock_guard<mutex> lock(m_MutexCustomerQue);
        for (const auto& ord : orders)
            m_CustomerQue.push(ord);
    }
    m_CondVar.notify_all();
}

// Thread function to handle customer demands
void CWeldingCompany::customerThread(ACustomer customer) {
    while (true) {
        AOrderList orderList = customer->waitForDemand();
        if (!orderList) {
            m_CustomersCount++; // This customer has no more demands
            break;
        }

        m_OrderCount++;
        unsigned orderId = orderList->m_MaterialID;

        {
            unique_lock<mutex> lockCache(m_MutexCache);
            // Create material info if it does not exist
            if (m_MaterialCache.find(orderId) == m_MaterialCache.end()) {
                m_MaterialCache[orderId] = make_shared<MaterialInfo>(orderId, m_Producers, *this);
                lockCache.unlock();

                // Request price list from all producers
                for (auto& prod : m_Producers)
                    prod->sendPriceList(orderId);
            }
        }

        WorkItem item{orderList, customer, orderId};
        {
            unique_lock<mutex> lockCache(m_MutexCache);
            m_MaterialCache[orderId]->addToBacklog(item);
            // if the Unified pricelist is ready, push for solving 
            if (m_MaterialCache[orderId]->isReady()) {
                vector<WorkItem> back = m_MaterialCache[orderId]->pushFromBacklog();
                lockCache.unlock();
                this->pushToCustomerQue(back);
            }
        }
    }
}

// Thread function to process work items
void CWeldingCompany::workerThread() {
  while (true) {
      {
          unique_lock<mutex> lock(m_MutexMain);
          m_CondVar.wait(lock, [&] {
              return m_Stop || !m_MaterialQue.empty() || !m_CustomerQue.empty();
          });
      }

      // Check for safely exiting the thread
      if (m_Stop && m_OrderCount == 0 && m_MaterialQue.empty() && m_CustomerQue.empty())
          break;

      m_ActiveWorkers++;

      {
          // Try to process a material update from the material queue
          unique_lock<mutex> lockMaterialQue(m_MutexMaterialQue);
          if (!m_MaterialQue.empty()) {
              unsigned id = m_MaterialQue.front();
              m_MaterialQue.pop();
              lockMaterialQue.unlock(); 

              {
                  // Pull backlog orders for the material and push to customer queue
                  unique_lock<mutex> lockCache(m_MutexCache);
                  vector<WorkItem> back = m_MaterialCache[id]->pushFromBacklog();
                  lockCache.unlock();
                  this->pushToCustomerQue(back);
              }
          }
      }

      {
          // Try to process a customer order from the customer queue
          unique_lock<mutex> lockCustomer(m_MutexCustomerQue);
          if (!m_CustomerQue.empty()) {
              WorkItem item = m_CustomerQue.front();
              m_CustomerQue.pop();
              lockCustomer.unlock(); 

              APriceList Uni;
              {
                  // Get the unified price list for the material from the cache
                  unique_lock<mutex> lockCache(m_MutexCache);
                  Uni = m_MaterialCache[item.idOrder]->getUniPriceList();
              }

              // Solve each weld
              for (auto& i : item.order->m_List) {
                  seqSolve(Uni, i);
              }

              // Notify the customer that the order is completed
              item.customer->completed(item.order);
              m_OrderCount--;
          }
      }

      // If all customers have been served, notify stop
      if (m_CustomersCount == static_cast<int>(m_Customers.size())) {
          m_StopCondVar.notify_all();
      }

      m_ActiveWorkers--;

      // Notify completion
      if (m_ActiveWorkers == 0 && m_MaterialQue.empty() && m_CustomerQue.empty())
          m_FinishCondVar.notify_all();
  }
}

// Wait for all threads to finish and stop the simulation
void CWeldingCompany::stop() {
    {
        unique_lock<mutex> lock(m_MutexStop);
        m_StopCondVar.wait(lock, [&] {
            return m_CustomersCount == static_cast<int>(m_Customers.size());
        });
        m_Stop = true;
    }

    m_CondVar.notify_all();

    {
        unique_lock<mutex> lock(m_MutexMain);
        m_FinishCondVar.wait(lock, [&] {
            return m_MaterialQue.empty() && m_CustomerQue.empty();
        });
    }

    for (auto& th : m_CustomerThreads) {
        if (th.joinable()) {
            th.join();
        }
    }

    for (auto& th : m_WorkerThreads) {
        if (th.joinable()) {
            th.join();
        }
    }
}

// ------------------ MaterialInfo class ------------------

// Check if the unified price list is ready
bool CWeldingCompany::MaterialInfo::isReady() const {
    return i_Ready;
}

// Add a work item to the backlog
void CWeldingCompany::MaterialInfo::addToBacklog(const WorkItem& order) {
    i_backlog.push_back(order);
}

// Push all backlog orders and clear the backlog
vector<CWeldingCompany::WorkItem> CWeldingCompany::MaterialInfo::pushFromBacklog() {
    vector<WorkItem> out = move(i_backlog);
    i_backlog.clear();
    return out;
}

// Add price list info from a producer
void CWeldingCompany::MaterialInfo::addPriceListInfo(const AProducer& producer, const APriceList& priceList) {
    for (auto& prod : priceList->m_List)
        i_Products.push_back(make_pair(true, prod));

    i_RemainingProducers.insert(producer);

    if (i_Producers.size() <= i_RemainingProducers.size()) {
        makeUniPriceList();
        i_Ready = true;
        i_Parent.pushToMaterialQue(i_ID);
        i_Parent.m_CondVar.notify_all();
    }
}

// Generate the unified price list by removing dominated products
void CWeldingCompany::MaterialInfo::makeUniPriceList() {
    for (size_t i = 0; i < i_Products.size(); i++) {
        for (size_t j = i + 1; j < i_Products.size(); j++) {
            if (i_Products[i].first == false) continue;

            bool sameDims = 
                (i_Products[i].second.m_H == i_Products[j].second.m_H && i_Products[i].second.m_W == i_Products[j].second.m_W) ||
                (i_Products[i].second.m_H == i_Products[j].second.m_W && i_Products[i].second.m_W == i_Products[j].second.m_H);

            if (sameDims) {
                if (i_Products[i].second.m_Cost > i_Products[j].second.m_Cost)
                    i_Products[i].first = false;
                else
                    i_Products[j].first = false;
            }
        }

        if (i_Products[i].first == true)
            UniPriceList->add(i_Products[i].second);
    }
}

// Return the unified price list
APriceList CWeldingCompany::MaterialInfo::getUniPriceList() const {
    return UniPriceList;
}

//-------------------------------------------------------------------------------------------------
int main()
{
  using namespace placeholders;
  CWeldingCompany  test;

  AProducer p1 = make_shared<CProducerSync> ( bind ( &CWeldingCompany::addPriceList, &test, _1, _2 ) );
  AProducerAsync p2 = make_shared<CProducerAsync> ( bind ( &CWeldingCompany::addPriceList, &test, _1, _2 ) );

  // AProducer p3 = make_shared<CProducerSync> ( bind ( &CWeldingCompany::addPriceList, &test, _1, _2 ) );
  // AProducerAsync p4 = make_shared<CProducerAsync> ( bind ( &CWeldingCompany::addPriceList, &test, _1, _2 ) );
  // AProducerAsync p5 = make_shared<CProducerAsync> ( bind ( &CWeldingCompany::addPriceList, &test, _1, _2 ) );
  // AProducerAsync p6 = make_shared<CProducerAsync> ( bind ( &CWeldingCompany::addPriceList, &test, _1, _2 ) );
  // AProducerAsync p7 = make_shared<CProducerAsync> ( bind ( &CWeldingCompany::addPriceList, &test, _1, _2 ) );
  // AProducerAsync p8 = make_shared<CProducerAsync> ( bind ( &CWeldingCompany::addPriceList, &test, _1, _2 ) );

  test . addProducer ( p1 );
  test . addProducer ( p2 );

  // test . addProducer ( p3 );
  // test . addProducer ( p4 );
  // test . addProducer ( p5 );
  // test . addProducer ( p6 );
  // test . addProducer ( p7 );
  // test . addProducer ( p8 );

  test . addCustomer ( make_shared<CCustomerTest> ( 2 ) );

  // test . addCustomer ( make_shared<CCustomerTest> ( 5 ) );
  // test . addCustomer ( make_shared<CCustomerTest> ( 1 ) );
  // test . addCustomer ( make_shared<CCustomerTest> ( 1 ) );
  // test . addCustomer ( make_shared<CCustomerTest> ( 1 ) );
  // test . addCustomer ( make_shared<CCustomerTest> ( 1 ) );

  p2 -> start ();
  
  // p4 -> start ();
  // p5 -> start ();
  // p6 -> start ();
  // p7 -> start ();
  // p8 -> start ();


  test . start ( 3 );
  test . stop ();
  p2 -> stop ();
  
  // p4 -> stop ();
  // p5 -> stop ();
  // p6 -> stop ();
  // p7 -> stop ();
  // p8 -> stop ();

  return EXIT_SUCCESS;
}