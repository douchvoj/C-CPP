#ifndef COMPONENTS_H
#define COMPONENTS_H

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <sstream> 
using namespace std;

class CComponent {
public:
    CComponent() = default;
    virtual ~CComponent() = default;
    virtual void print(ostream& os, const string& indent) const = 0;
    virtual CComponent* clone() const = 0;
};

class CCPU : public CComponent {
public:
    CCPU(int cores, int frequency);
    CCPU(const CCPU& other);
    CCPU* clone() const override;
    void print(ostream& os, const string& indent) const override;

private:
    int c_cores;
    int c_frequency;
};

class CMemory : public CComponent {
public:
    CMemory(int size);
    CMemory(const CMemory& other);
    CMemory* clone() const override;
    void print(ostream& os, const string& indent) const override;

private:
    int c_memorySize;
};

class CDisk : public CComponent {
public:
    enum DiskType { SSD, MAGNETIC };

    CDisk(DiskType type, int size);
    CDisk(const CDisk& other);
    CDisk* clone() const override;
    CDisk& addPartition(int size, const string& id);
    void print(ostream& os, const string& indent) const override;

private:
    void print_partitions(ostream& os, const string& indent) const;

    DiskType c_type;
    int c_sizeOfDisk;
    vector<pair<int, string>> c_partitions;
};
class CComputer{
    public:
      CComputer();
      CComputer(const string & address);
      ~CComputer()=default;
      CComputer& operator=( CComputer & other);
      CComputer(const CComputer & other);
      CComputer& addAddress(const string & address);
      CComputer& addComponent(const CComponent & x);
      CComputer* clone();
      friend ostream& operator<<(ostream& os, const CComputer& net);
      string get_address()const;
      void print(ostream & os,const string & indent)const;

    protected:
      vector<string> c_address;            
      vector<unique_ptr<CComponent>> c_pcs;    
};
class CNetwork{
    public:
      CNetwork();
      CNetwork(const string & name);
      CNetwork(const CNetwork & other);
      ~CNetwork()=default;
      CNetwork& operator=( CNetwork & other);
      CNetwork& addComputer( CComputer comp);
      CComputer* findComputer(const string & nameTofind);
      friend ostream& operator<<(ostream& os, const CNetwork& net);
    protected: 
      string c_name;
      vector<unique_ptr<CComputer>> c_Computers;  
};
#endif // COMPONENTS_H
