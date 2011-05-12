// -*- C++ -*-
//
// Package:     FwkIO
// Class  :     DQMRootSource
// 
// Implementation:
//     [Notes on implementation]
//
// Original Author:  Chris Jones
//         Created:  Tue May  3 11:13:47 CDT 2011
// $Id: DQMRootSource.cc,v 1.4 2011/05/12 00:22:55 chrjones Exp $
//

// system include files
#include <vector>
#include <string>
#include <map>
#include <list>
#include <set>
#include "TFile.h"
#include "TTree.h"
#include "TString.h"
#include "TH1.h"
#include "TH2.h"
#include "TProfile.h"

// user include files
#include "FWCore/Framework/interface/InputSource.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DQMServices/Core/interface/DQMStore.h"
#include "DQMServices/Core/interface/MonitorElement.h"
#include "FWCore/ServiceRegistry/interface/Service.h"

#include "FWCore/Framework/interface/RunPrincipal.h"
#include "FWCore/Framework/interface/LuminosityBlockPrincipal.h"

#include "FWCore/Framework/interface/Run.h"
#include "FWCore/Framework/interface/LuminosityBlock.h"

#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"

#include "FWCore/Framework/interface/InputSourceMacros.h"
#include "FWCore/Framework/interface/FileBlock.h"

#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "format.h"

namespace {
  //adapter functions
  MonitorElement* createElement(DQMStore& iStore, const char* iName, TH1F* iHist) {
    //std::cout <<"create: hist size "<<iName <<" "<<iHist->GetEffectiveEntries()<<std::endl;
    return iStore.book1D(iName, iHist);
  }
  //NOTE: the merge logic comes from DataFormats/Histograms/interface/MEtoEDMFormat.h
  void mergeTogether(TH1* iOriginal,TH1* iToAdd) {
    if(iOriginal->TestBit(TH1::kCanRebin)==true && iToAdd->TestBit(TH1::kCanRebin) ==true) {
      TList list;
      list.Add(iToAdd);
      if( -1 == iOriginal->Merge(&list)) {
        edm::LogError("MergeFailure")<<"Failed to merge DQM element "<<iOriginal->GetName();
      }
    } else {
      if (iOriginal->GetNbinsX() == iToAdd->GetNbinsX() &&
          iOriginal->GetXaxis()->GetXmin() == iToAdd->GetXaxis()->GetXmin() &&
          iOriginal->GetXaxis()->GetXmax() == iToAdd->GetXaxis()->GetXmax() &&
          iOriginal->GetNbinsY() == iToAdd->GetNbinsY() &&
          iOriginal->GetYaxis()->GetXmin() == iToAdd->GetYaxis()->GetXmin() &&
          iOriginal->GetYaxis()->GetXmax() == iToAdd->GetYaxis()->GetXmax() &&
          iOriginal->GetNbinsZ() == iToAdd->GetNbinsZ() &&
          iOriginal->GetZaxis()->GetXmin() == iToAdd->GetZaxis()->GetXmin() &&
          iOriginal->GetZaxis()->GetXmax() == iToAdd->GetZaxis()->GetXmax()) {
          iOriginal->Add(iToAdd);
      } else {
        edm::LogError("MergeFailure")<<"Found histograms with different axis limitsm '"<<iOriginal->GetName()<<"' not merged.";
      }
    } 
  }
  void mergeWithElement(MonitorElement* iElement, TH1F* iHist) {
    //std::cout <<"merge: hist size "<<iElement->getName() <<" "<<iHist->GetEffectiveEntries()<<std::endl;
    mergeTogether(iElement->getTH1F(),iHist);
  }
  MonitorElement* createElement(DQMStore& iStore, const char* iName, TH1S* iHist) {
    return iStore.book1S(iName, iHist);
  }
  void mergeWithElement(MonitorElement* iElement, TH1S* iHist) {
    mergeTogether(iElement->getTH1S(),iHist);
  }  
  MonitorElement* createElement(DQMStore& iStore, const char* iName, TH1D* iHist) {
    return iStore.book1DD(iName, iHist);
  }
  void mergeWithElement(MonitorElement* iElement, TH1D* iHist) {
    mergeTogether(iElement->getTH1D(),iHist);
  }
  MonitorElement* createElement(DQMStore& iStore, const char* iName, TH2F* iHist) {
    return iStore.book2D(iName, iHist);
  }
  void mergeWithElement(MonitorElement* iElement, TH2F* iHist) {
    mergeTogether(iElement->getTH2F(),iHist);
  }
  MonitorElement* createElement(DQMStore& iStore, const char* iName, TH2S* iHist) {
    return iStore.book2S(iName, iHist);
  }
  void mergeWithElement(MonitorElement* iElement, TH2S* iHist) {
    mergeTogether(iElement->getTH2S(),iHist);
  }  
  MonitorElement* createElement(DQMStore& iStore, const char* iName, TH2D* iHist) {
    return iStore.book2DD(iName, iHist);
  }
  void mergeWithElement(MonitorElement* iElement, TH2D* iHist) {
    mergeTogether(iElement->getTH2D(),iHist);
  }
  MonitorElement* createElement(DQMStore& iStore, const char* iName, TH3F* iHist) {
    return iStore.book3D(iName, iHist);
  }
  void mergeWithElement(MonitorElement* iElement, TH3F* iHist) {
    mergeTogether(iElement->getTH3F(),iHist);
  }
  MonitorElement* createElement(DQMStore& iStore, const char* iName, TProfile* iHist) {
    return iStore.bookProfile(iName, iHist);
  }
  void mergeWithElement(MonitorElement* iElement, TProfile* iHist) {
    mergeTogether(iElement->getTProfile(),iHist);
  }
  MonitorElement* createElement(DQMStore& iStore, const char* iName, TProfile2D* iHist) {
    return iStore.bookProfile2D(iName, iHist);
  }
  void mergeWithElement(MonitorElement* iElement, TProfile2D* iHist) {
    mergeTogether(iElement->getTProfile2D(),iHist);
  }

  MonitorElement* createElement(DQMStore& iStore, const char* iName, Long64_t& iValue) {
    MonitorElement* e = iStore.bookInt(iName);
    e->Fill(iValue);
    return e;
  }
  
  //NOTE: the merge logic comes from DataFormats/Histograms/interface/MEtoEDMFormat.h
  void mergeWithElement(MonitorElement* iElement, Long64_t& iValue) {
    const std::string& name = iElement->getFullname();
    if(name.find("EventInfo/processedEvents") != std::string::npos) {
      iElement->Fill(iValue+iElement->getIntValue());
    } else {
      if(name.find("EventInfo/iEvent") != std::string::npos ||
         name.find("EventInfo/iLumiSection") != std::string::npos) {
        if(iValue > iElement->getIntValue()) {
             iElement->Fill(iValue);
        }
      }
    }
  }

  MonitorElement* createElement(DQMStore& iStore, const char* iName, double& iValue) {
    MonitorElement* e = iStore.bookFloat(iName);
    e->Fill(iValue);
    return e;
  }
  void mergeWithElement(MonitorElement* iElement, double& iValue) {
    //no merging    
  }
  MonitorElement* createElement(DQMStore& iStore, const char* iName, TString* iValue) {
    return iStore.bookString(iName,iValue->Data());
  }
  void mergeWithElement(MonitorElement* iElement, TString* TString) {
    //no merging    
  }
  
  void splitName(const std::string& iFullName, std::string& oPath,const char*& oName) {
    oPath = iFullName;
    size_t index = oPath.find_last_of('/');
    if(index == std::string::npos) {
      oPath = std::string();
      oName = iFullName.c_str();
    } else {
      oPath.resize(index);
      oName = iFullName.c_str()+index+1;
    }
  }
  
  struct RunLumiToRange { 
    unsigned int m_run, m_lumi;
    ULong64_t m_firstIndex, m_lastIndex; //last is inclusive
    unsigned int m_type; //A value in TypeIndex 
  };
  
  class TreeReaderBase {
  public:
    TreeReaderBase() {}
    virtual ~TreeReaderBase() {}
    
    MonitorElement* read(ULong64_t iIndex, DQMStore& iStore, bool iIsLumi){
      return doRead(iIndex,iStore,iIsLumi);
    }
    virtual void setTree(TTree* iTree) =0;
  protected:
    TTree* m_tree;
  private:
    virtual MonitorElement* doRead(ULong64_t iIndex, DQMStore& iStore, bool iIsLumi)=0;
  };
  
  template<class T>
  class TreeObjectReader: public TreeReaderBase {
  public:
    TreeObjectReader():m_tree(0),m_fullName(0),m_buffer(0),m_tag(0){
    }
    virtual MonitorElement* doRead(ULong64_t iIndex, DQMStore& iStore, bool iIsLumi) {
      m_tree->GetEntry(iIndex);
      MonitorElement* element = iStore.get(*m_fullName);
      if(0 == element) {
        std::string path;
        const char* name;
        splitName(*m_fullName, path,name);
        iStore.cd(path);
         element = createElement(iStore,name,m_buffer);
        if(iIsLumi) { element->setLumiFlag();}
      } else {
        mergeWithElement(element,m_buffer);
      }
      if(0!= m_tag) {
        iStore.tag(element,m_tag);
      }
      return element;
    }
    virtual void setTree(TTree* iTree)  {
      m_tree = iTree;
      m_tree->SetBranchAddress(kFullNameBranch,&m_fullName);
      m_tree->SetBranchAddress(kFlagBranch,&m_tag);
      m_tree->SetBranchAddress(kValueBranch,&m_buffer);
    }
  private:
    TTree* m_tree;
    std::string* m_fullName;
    T* m_buffer;
    uint32_t m_tag;
  };
  
  template<class T>
  class TreeSimpleReader : public TreeReaderBase {
  public:
    TreeSimpleReader():m_tree(0),m_fullName(0),m_buffer(),m_tag(0){
    }
    virtual MonitorElement* doRead(ULong64_t iIndex, DQMStore& iStore,bool iIsLumi) {
      m_tree->GetEntry(iIndex);
      MonitorElement* element = iStore.get(*m_fullName);
      if(0 == element) {
        std::string path;
        const char* name;
        splitName(*m_fullName, path,name);
        iStore.cd(path);
        element = createElement(iStore,name,m_buffer);
        if(iIsLumi) { element->setLumiFlag();}
      } else {
        mergeWithElement(element, m_buffer);
      }
      if(0!=m_tag) {
        iStore.tag(element,m_tag);
      }
      return element;
    }
    virtual void setTree(TTree* iTree)  {
      m_tree = iTree;
      m_tree->SetBranchAddress(kFullNameBranch,&m_fullName);
      m_tree->SetBranchAddress(kFlagBranch,&m_tag);
      m_tree->SetBranchAddress(kValueBranch,&m_buffer);
    }
  private:
    TTree* m_tree;
    std::string* m_fullName;
    T m_buffer;
    uint32_t m_tag;
  };
  
}

class DQMRootSource : public edm::InputSource
{

   public:
      DQMRootSource(edm::ParameterSet const&, const edm::InputSourceDescription&);
      virtual ~DQMRootSource();

      // ---------- const member functions ---------------------

      // ---------- static member functions --------------------

      // ---------- member functions ---------------------------
      static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);
   protected:
     virtual void endLuminosityBlock(edm::LuminosityBlock&);
     virtual void endRun(edm::Run&);
     
   private:
      DQMRootSource(const DQMRootSource&); // stop default

      virtual edm::InputSource::ItemType getNextItemType();
      //NOTE: the following is really read next run auxiliary
      virtual boost::shared_ptr<edm::RunAuxiliary> readRunAuxiliary_() ;
      virtual boost::shared_ptr<edm::LuminosityBlockAuxiliary> readLuminosityBlockAuxiliary_() ;
      virtual boost::shared_ptr<edm::RunPrincipal> readRun_(boost::shared_ptr<edm::RunPrincipal> rpCache);
      virtual boost::shared_ptr<edm::LuminosityBlockPrincipal> readLuminosityBlock_( boost::shared_ptr<edm::LuminosityBlockPrincipal> lbCache);
      virtual edm::EventPrincipal* readEvent_() ;
      
      virtual boost::shared_ptr<edm::FileBlock> readFile_();
      virtual void closeFile_();
      
      void readNextItemType();
      void setupFile(unsigned int iIndex);
      void readElements();
      
      const DQMRootSource& operator=(const DQMRootSource&); // stop default

      // ---------- member data --------------------------------
      std::vector<std::string> m_fileNames;
      edm::RunAuxiliary m_runAux;
      edm::LuminosityBlockAuxiliary m_lumiAux;
      edm::InputSource::ItemType m_nextItemType;

      size_t m_fileIndex;
      std::list<unsigned int>::iterator m_nextIndexItr;
      std::list<unsigned int>::iterator m_presentIndexItr;
      std::vector<RunLumiToRange> m_runlumiToRange;
      std::auto_ptr<TFile> m_file;
      std::vector<TTree*> m_trees;
      std::vector<boost::shared_ptr<TreeReaderBase> > m_treeReaders;
      
      std::list<unsigned int> m_orderedIndices;
      unsigned int m_lastSeenRun;
      bool m_doNotReadRemainingPartsOfFileSinceFrameworkTerminating;
      std::set<MonitorElement*> m_lumiElements;
};

//
// constants, enums and typedefs
//

//
// static data member definitions
//

void 
DQMRootSource::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  desc.addUntracked<std::vector<std::string> >("fileNames");
  descriptions.addDefault(desc);
}
//
// constructors and destructor
//
DQMRootSource::DQMRootSource(edm::ParameterSet const& iPSet, const edm::InputSourceDescription& iDesc):
edm::InputSource(iPSet,iDesc),
m_fileNames(iPSet.getUntrackedParameter<std::vector<std::string> >("fileNames")),
m_nextItemType(edm::InputSource::IsFile),
m_fileIndex(0),
m_trees(kNIndicies,static_cast<TTree*>(0)),
m_treeReaders(kNIndicies,boost::shared_ptr<TreeReaderBase>()),
m_lastSeenRun(0),
m_doNotReadRemainingPartsOfFileSinceFrameworkTerminating(false)
{
  if(m_fileIndex ==m_fileNames.size()) {
    m_nextItemType=edm::InputSource::IsStop;
  } else{
    m_treeReaders[kIntIndex].reset(new TreeSimpleReader<Long64_t>());
    m_treeReaders[kFloatIndex].reset(new TreeSimpleReader<double>());
    m_treeReaders[kStringIndex].reset(new TreeObjectReader<TString>());
    m_treeReaders[kTH1FIndex].reset(new TreeObjectReader<TH1F>());
    m_treeReaders[kTH1SIndex].reset(new TreeObjectReader<TH1S>());
    m_treeReaders[kTH1DIndex].reset(new TreeObjectReader<TH1D>());
    m_treeReaders[kTH2FIndex].reset(new TreeObjectReader<TH2F>());
    m_treeReaders[kTH2SIndex].reset(new TreeObjectReader<TH2S>());
    m_treeReaders[kTH2DIndex].reset(new TreeObjectReader<TH2D>());
    m_treeReaders[kTH3FIndex].reset(new TreeObjectReader<TH3F>());
    m_treeReaders[kTProfileIndex].reset(new TreeObjectReader<TProfile>());
    m_treeReaders[kTProfile2DIndex].reset(new TreeObjectReader<TProfile2D>());
  }
  
}

// DQMRootSource::DQMRootSource(const DQMRootSource& rhs)
// {
//    // do actual copying here;
// }

DQMRootSource::~DQMRootSource()
{
}

//
// assignment operators
//
// const DQMRootSource& DQMRootSource::operator=(const DQMRootSource& rhs)
// {
//   //An exception safe implementation is
//   DQMRootSource temp(rhs);
//   swap(rhs);
//
//   return *this;
// }

//
// member functions
//
namespace {
  struct no_deleter {
    void operator()(void*) {}
  };
}

edm::EventPrincipal*
DQMRootSource::readEvent_() 
{
  return 0;
}

edm::InputSource::ItemType 
DQMRootSource::getNextItemType()
{
  //std::cout <<"getNextItemType "<<m_nextItemType<<std::endl;
  return m_nextItemType;
}
boost::shared_ptr<edm::RunAuxiliary> 
DQMRootSource::readRunAuxiliary_()
{
  //std::cout <<"readRunAuxiliary_"<<std::endl;
  assert(m_nextIndexItr != m_orderedIndices.end());
  const RunLumiToRange runLumiRange = m_runlumiToRange[*m_nextIndexItr];

  m_runAux.id() = edm::RunID(runLumiRange.m_run);    
  return boost::shared_ptr<edm::RunAuxiliary>( new edm::RunAuxiliary(m_runAux) );
}
boost::shared_ptr<edm::LuminosityBlockAuxiliary> 
DQMRootSource::readLuminosityBlockAuxiliary_()
{
  //std::cout <<"readLuminosityBlockAuxiliary_"<<std::endl;
  assert(m_nextIndexItr != m_orderedIndices.end());
  const RunLumiToRange runLumiRange = m_runlumiToRange[*m_nextIndexItr];
  m_lumiAux.id() = edm::LuminosityBlockID(runLumiRange.m_run,runLumiRange.m_lumi);
  
  return boost::shared_ptr<edm::LuminosityBlockAuxiliary>(new edm::LuminosityBlockAuxiliary(m_lumiAux));
}
boost::shared_ptr<edm::RunPrincipal> 
DQMRootSource::readRun_(boost::shared_ptr<edm::RunPrincipal> rpCache) 
{
  readNextItemType();
  m_lastSeenRun = rpCache->id().run();
  //std::cout <<"readRun_"<<std::endl;
  return rpCache;
}
boost::shared_ptr<edm::LuminosityBlockPrincipal> 
DQMRootSource::readLuminosityBlock_( boost::shared_ptr<edm::LuminosityBlockPrincipal> lbCache) 
{
  //NOTE: need to reset all lumi block elements at this point
  for(std::set<MonitorElement*>::iterator it = m_lumiElements.begin(), itEnd = m_lumiElements.end();
      it != itEnd;
      ++it) {
      (*it)->Reset();
  }
  readNextItemType();
  //std::cout <<"readLuminosityBlock_"<<std::endl;
  return lbCache;
}

boost::shared_ptr<edm::FileBlock>
DQMRootSource::readFile_() {
  //std::cout <<"readFile_"<<std::endl;
  setupFile(m_fileIndex);
  ++m_fileIndex;
  readNextItemType();
  
  m_doNotReadRemainingPartsOfFileSinceFrameworkTerminating = false;
  return boost::shared_ptr<edm::FileBlock>(new edm::FileBlock);
}


void 
DQMRootSource::endLuminosityBlock(edm::LuminosityBlock& iLumi) {
  //std::cout <<"DQMRootSource::endLumi"<<std::endl;
  RunLumiToRange runLumiRange = m_runlumiToRange[*m_presentIndexItr];
  if(runLumiRange.m_run == iLumi.id().run() &&
     runLumiRange.m_lumi == iLumi.id().luminosityBlock()) {
       readElements();
  }
}
void 
DQMRootSource::endRun(edm::Run& iRun){
  //std::cout <<"DQMRootSource::endRun"<<std::endl;
  RunLumiToRange runLumiRange = m_runlumiToRange[*m_presentIndexItr];
  //NOTE: it is possible to have an endRun when all we have stored is lumis
  if(runLumiRange.m_lumi == 0 && 
     runLumiRange.m_run == iRun.id().run()) {
    readElements();
  }
  //NOTE: the framework will call endRun before closeFile in the case
  // where the frameworks is terminating
  m_doNotReadRemainingPartsOfFileSinceFrameworkTerminating=true;
}


void
DQMRootSource::closeFile_() {
  //std::cout <<"closeFile_"<<std::endl;
  //when going from one file to the next the framework does not call
  // 'endRun' or 'endLumi' until it looks to see if the other file contains
  // a new run or lumi. If the other file doesn't then  
  if(not m_doNotReadRemainingPartsOfFileSinceFrameworkTerminating) {
    while(m_presentIndexItr != m_orderedIndices.end()) {
      readElements();
    }
  }
}

void 
DQMRootSource::readElements() {
  edm::Service<DQMStore> store;
  RunLumiToRange runLumiRange = m_runlumiToRange[*m_presentIndexItr];
  bool shouldContinue = false;
  do {
    shouldContinue = false;
    boost::shared_ptr<TreeReaderBase> reader = m_treeReaders[runLumiRange.m_type];
    for(ULong64_t index = runLumiRange.m_firstIndex, endIndex=runLumiRange.m_lastIndex+1;
    index != endIndex;
    ++index) {
      bool isLumi = runLumiRange.m_lumi !=0;
      MonitorElement* element = reader->read(index,*store,isLumi);
      if(isLumi) {
        m_lumiElements.insert(element);
      }
    }
    ++m_presentIndexItr;
    if(m_presentIndexItr != m_orderedIndices.end()) {
      //are there more parts to this same run/lumi?
      const RunLumiToRange nextRunLumiRange = m_runlumiToRange[*m_presentIndexItr];
      //continue to the next item if that item is either
      if( (nextRunLumiRange.m_run == runLumiRange.m_run) && 
          (nextRunLumiRange.m_lumi == runLumiRange.m_lumi) ) {
           shouldContinue= true;
           runLumiRange = nextRunLumiRange;
      } 
    }
  }while(shouldContinue);
}


void 
DQMRootSource::readNextItemType()
{
  //Do the work of actually figuring out where next to go
  RunLumiToRange runLumiRange = m_runlumiToRange[*m_nextIndexItr];
  if(m_nextItemType !=edm::InputSource::IsFile) {
    assert(m_nextIndexItr != m_orderedIndices.end());

    if(runLumiRange.m_lumi ==0) {
      //std::cout <<"reading run "<<runLumiRange.m_run<<std::endl;
      m_runAux.id() = edm::RunID(runLumiRange.m_run);    
    } else {
      if(m_nextItemType == edm::InputSource::IsRun) {
        //std::cout <<" proceeding with dummy run";
        m_nextItemType = edm::InputSource::IsLumi;
        return;
      }
      //std::cout <<"reading lumi "<<runLumiRange.m_run<<","<<runLumiRange.m_lumi<<std::endl;
      m_lumiAux.id() = edm::LuminosityBlockID(runLumiRange.m_run,runLumiRange.m_lumi);
    }
    ++m_nextIndexItr;
  } else {
    //NOTE: the following causes the iterator to move to before 'begin' but that is OK
    // since the first thing in the 'do while' loop is to advance the iterator which puts
    // us at the first entry in the file
    runLumiRange.m_run=0;
  }
  
  bool shouldContinue = false;
  do {
    shouldContinue = false;
    if(m_nextIndexItr == m_orderedIndices.end()) {
      //go to next file
      m_nextItemType = edm::InputSource::IsFile;
      //std::cout <<"going to next file"<<std::endl;
      if(m_fileIndex == m_fileNames.size()) {
        m_nextItemType = edm::InputSource::IsStop;
      }       
      break;
    }
    const RunLumiToRange nextRunLumiRange = m_runlumiToRange[*m_nextIndexItr];
    //continue to the next item if that item is either
    // 1) the same run or lumi as we just did or
    // 2) it is the run for the lumis we just processed (since this is technically an 'endrun' and sources do not signal those)
    if( (nextRunLumiRange.m_run == runLumiRange.m_run) && (
         nextRunLumiRange.m_lumi == runLumiRange.m_lumi || nextRunLumiRange.m_lumi ==0) ) {
         shouldContinue= true;
         runLumiRange = nextRunLumiRange;
         ++m_nextIndexItr;
    } 
    
  } while(shouldContinue);
  
  if(m_nextIndexItr != m_orderedIndices.end()) {
    if(m_runlumiToRange[*m_nextIndexItr].m_lumi == 0 && m_lastSeenRun != m_runlumiToRange[*m_nextIndexItr].m_run) {
      m_nextItemType = edm::InputSource::IsRun;
    } else {
      if(m_runlumiToRange[*m_nextIndexItr].m_run != m_lastSeenRun) {
        //we have to create a dummy Run since we switched to a lumi in a new run
        m_nextItemType = edm::InputSource::IsRun;
      } else {
        m_nextItemType = edm::InputSource::IsLumi;      
      }
    }
  }
}

void 
DQMRootSource::setupFile(unsigned int iIndex)
{
  
  m_file = std::auto_ptr<TFile>(TFile::Open(m_fileNames[iIndex].c_str()));
  TTree* indicesTree = dynamic_cast<TTree*>(m_file->Get(kIndicesTree));
  assert(0!=indicesTree);
  
  m_runlumiToRange.clear();
  m_runlumiToRange.reserve(indicesTree->GetEntries());
  m_orderedIndices.clear();

  RunLumiToRange temp;
  indicesTree->SetBranchAddress(kRunBranch,&temp.m_run);
  indicesTree->SetBranchAddress(kLumiBranch,&temp.m_lumi);
  indicesTree->SetBranchAddress(kTypeBranch,&temp.m_type);
  indicesTree->SetBranchAddress(kFirstIndex,&temp.m_firstIndex);
  indicesTree->SetBranchAddress(kLastIndex,&temp.m_lastIndex);

  //Need to reorder items since if there was a merge done the same  Run and/or Lumi can appear multiple times
  // but we want to process them all at once
  
  //We use a std::list for m_orderedIndices since inserting into the middle of a std::list does not 
  // disrupt the iterators to already existing entries
  
  //The Map is used to see if a Run/Lumi pair has appeared before
  typedef std::map<std::pair<unsigned int, unsigned int>, std::list<unsigned int>::iterator > RunLumiToLastEntryMap;
  RunLumiToLastEntryMap runLumiToLastEntryMap;
  
  std::list<unsigned int>::iterator positionOfFirstIndexForRun = m_orderedIndices.end();
  unsigned int lastSeenRun = 0;
  for(Long64_t index = 0; index != indicesTree->GetEntries();++index) {
    indicesTree->GetEntry(index);
    m_runlumiToRange.push_back(temp);
    
    std::pair<unsigned int, unsigned int> runLumi(temp.m_run,temp.m_lumi);
    
    RunLumiToLastEntryMap::iterator itFind = runLumiToLastEntryMap.find(runLumi);
    if(itFind == runLumiToLastEntryMap.end()) {
      //does not already exist
      std::list<unsigned int>::iterator iter = m_orderedIndices.insert(m_orderedIndices.end(),index);
      runLumiToLastEntryMap[runLumi]=iter;
      if(lastSeenRun != temp.m_run) {
        lastSeenRun = temp.m_run;
        positionOfFirstIndexForRun = iter;
      }
    } else {
      //We need to do a merge since the run/lumi already appeared. Put it after the existing entry
      std::list<unsigned int>::iterator iter = m_orderedIndices.insert(itFind->second,index);
      itFind->second = iter;
    }
  }
  m_nextIndexItr = m_orderedIndices.begin();
  m_presentIndexItr = m_orderedIndices.begin();
  
  if(m_nextIndexItr != m_orderedIndices.end()) {
    for( size_t index = 0; index < kNIndicies; ++index) {
      m_trees[index] = dynamic_cast<TTree*>(m_file->Get(kTypeNames[index]));
      assert(0!=m_trees[index]);
      m_treeReaders[index]->setTree(m_trees[index]);
    }
  }
  //After a file open, the framework expects to see a new 'IsRun'
  m_lastSeenRun = 0;
}

//
// const member functions
//

//
// static member functions
//
DEFINE_FWK_INPUT_SOURCE(DQMRootSource);