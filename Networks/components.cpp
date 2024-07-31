#include "components.h"

// CCPU
//--------------------------------------------------------------------------------------------------------------------------------------------
CCPU::CCPU(const int cores,const int frequency)
    :c_cores(cores),c_frequency(frequency)
{}
CCPU::CCPU (const CCPU & other)
    :c_cores(other.c_cores),c_frequency(other.c_frequency)
{}
CCPU* CCPU::clone() const {
    return new CCPU(*this);
}
void CCPU::print(ostream & os,const string & indent)const {
    os << indent << "-CPU, " << c_cores << " cores @ " << c_frequency <<"MHz"<< endl;
}


// CMemory
//--------------------------------------------------------------------------------------------------------------------------------------------
CMemory::CMemory(int size)
    : c_memorySize(size) 
{}
CMemory::CMemory(const CMemory& other)
    : c_memorySize(other.c_memorySize) 
{}
CMemory* CMemory::clone() const {
    return new CMemory(*this);
}
void CMemory::print(ostream& os, const string& indent) const {
    os << indent << "-Memory, " << c_memorySize << " MiB" << endl;
}


// CDisk 
//--------------------------------------------------------------------------------------------------------------------------------------------
CDisk::CDisk(DiskType type,int size)
    :c_type(type),c_sizeOfDisk(size)
{}
CDisk::CDisk (const CDisk & other)
    :c_type(other.c_type), c_sizeOfDisk(other.c_sizeOfDisk)
{
    for(const auto & i: other.c_partitions) c_partitions.push_back(make_pair(i.first, i.second));
}
CDisk * CDisk::clone()const {
    return new CDisk(*this);
}
CDisk& CDisk::addPartition(const int size, const string & id){
    c_partitions.push_back(make_pair(size,id));
    return *this;
}
void CDisk::print_partitions(ostream & os, const string & indent)const{
    int x =0;
    for(auto & i: c_partitions){
        if(i!=c_partitions.back())os << indent << "+-["<<x<<"]: " << i.first << " GiB, " << i.second << endl;
        else os << indent << "\\-["<<x<<"]: " << i.first << " GiB, " << i.second << endl;
        x++;
    }
}
void CDisk::print(ostream & os,const string & indent)const {
    os <<indent<<(c_type == DiskType::SSD ? "-SSD, " : "-HDD, ") <<c_sizeOfDisk << " GiB" << endl;
    string indent1 = indent;
    indent1.erase(indent1.size()-1,indent1.size()-1);
    if((*indent1.begin()=='+' ||*indent1.begin()=='\\')&&indent1.size()>1)indent1.erase(*indent1.begin(),*indent1.begin());
    else if((*indent1.begin()=='+' ||*indent1.begin()=='\\')&&indent1.size()==1)indent1="";

    if(indent[indent.size()-1]=='+')print_partitions(os,indent1 + "| ");
    else print_partitions(os,indent1 + "  ");
}


// CComputer
//--------------------------------------------------------------------------------------------------------------------------------------------
CComputer::CComputer(const string & address){
    c_address.push_back(address);
}
CComputer& CComputer::operator=( CComputer & other){
    c_address.clear();
    c_pcs.clear();
    for(auto & i:other.c_address)c_address.push_back(i);
    for(auto & i:other.c_pcs)c_pcs.emplace_back(i->clone());
    return *this;
}
CComputer::CComputer(const CComputer & other){
    for(const auto & i:other.c_address)c_address.push_back(i);
    for(const auto & i:other.c_pcs)c_pcs.emplace_back(i->clone());
}
CComputer& CComputer::addAddress(const string & address){
    c_address.push_back(address);
    return *this;
}
CComputer& CComputer::addComponent(const CComponent & x){
    c_pcs.emplace_back(x.clone());
    return *this;
}
CComputer* CComputer::clone(){ return new CComputer(*this);}
void CComputer::print(ostream & os,const string & indent)const{
    for(auto & i: c_address)if(i!=*c_address.begin())os <<indent << "+-"<<i << endl;
    for(auto & i: c_pcs){
    if(i!=c_pcs.back()){
        i->print(os,indent + "+");
    }
    else i->print(os,indent +"\\");
    }
}
string CComputer::get_address()const{
return *c_address.begin();
}
ostream& operator<<(ostream& os, const CComputer& net){
    os <<"Host: " <<net.get_address()<< endl;
    net.print(os,"");
    return os;
}

// CNetwork
//--------------------------------------------------------------------------------------------------------------------------------------------
CNetwork::CNetwork(const string & name)
    :c_name(name)
{}
CNetwork::CNetwork(const CNetwork & other){
    c_Computers.clear();
    c_name=other.c_name;
    for(auto & i:other.c_Computers)c_Computers.emplace_back(i->clone());
}
CNetwork& CNetwork::operator=( CNetwork & other){
    c_Computers.clear();
    c_name=other.c_name;
    for(auto & i:other.c_Computers)c_Computers.emplace_back(i->clone());
    return *this;
}
CNetwork& CNetwork::addComputer( CComputer comp){
    c_Computers.emplace_back(comp.clone());
    return *this;
}
CComputer* CNetwork::findComputer(const string & nameTofind){
    for(const auto& i:c_Computers){
        if(nameTofind==i->get_address())return i.get();
    }
    return nullptr;
}
ostream& operator<<(ostream& os, const CNetwork& net){
    os <<"Network: " << net.c_name << endl;
    for(auto & i:net.c_Computers){
        if(i!=net.c_Computers.back()){
            os<<"+-Host: " <<i->get_address() << endl;
            i->print(os,"| ");
        }
        else {
            os<<"\\-Host: " <<i->get_address() << endl;
            i->print(os,"  ");
        }
    }
    return os;
}