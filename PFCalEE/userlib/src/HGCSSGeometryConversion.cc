#include "HGCSSGeometryConversion.hh"
#include <sstream>
#include <iostream>
#include <cmath>

HGCSSGeometryConversion::HGCSSGeometryConversion(const unsigned & model, const double & cellsize, const bool bypassR, const unsigned nSiLayers){

  dopatch_=false;
  width_ = 200;//mm
  model_ = model;
  if (model==1) width_ = 500;
  else if (model == 2) width_ = 1700*2;
  else if (model == 3) width_ = 1000;
  else if (model == 4) width_ = 1300;
  
  cellSize_ = cellsize;
  bypassRadius_ = bypassR;
  nSiLayers_ = nSiLayers;

}
/*
unsigned HGCSSGeometryConversion::getNumberOfSiLayers(const DetectorEnum type,
						      const double & eta) const
{
  if (model_ != 2) return 3;
  if (type == DetectorEnum::FHCAL) return 3;
  unsigned etaBin = 0;
  if (fabs(eta)>=1.4 && fabs(eta)<=1.75) etaBin = 1;
  else if (fabs(eta)>1.75 && fabs(eta)<=2.15) etaBin = 2;
  else if (fabs(eta) > 2.15) etaBin = 3;
  if (etaBin==0){
    if (type == DetectorEnum::FECAL) return 2;
    else if (type == DetectorEnum::MECAL) return 2;
    else if (type == DetectorEnum::BECAL) return 2;
  }
  else {
    if (etaBin==1) return 3;
    else if (etaBin==2) return 2;
    else return 1;
  }
  return 3;
  }*/

unsigned HGCSSGeometryConversion::getNumberOfSiLayers(const DetectorEnum type,
						      const double & radius) const
{
  if (theDetector().subDetectorByEnum(type).isScint) return 3;
  if (model_ != 2) return nSiLayers_;

  double r1 = 1200;
  if (type == DetectorEnum::FHCAL) {
    r1 = 1000;
  }
  double r2 = theDetector().subDetectorByEnum(type).radiusLim;
  if (radius>r1) return 3;
  else if (radius>r2) return 2;
  else return 1;
  return 3;
}

HGCSSGeometryConversion::~HGCSSGeometryConversion(){
  std::map<DetectorEnum,std::vector<TH2Poly *> >::iterator liter =
    HistMapE_.begin();
  for (; liter !=HistMapE_.end();++liter){
    deleteHistos(liter->second);
  }
  HistMapE_.clear();

  liter = HistMapTime_.begin();
  for (; liter !=HistMapTime_.end();++liter){
    deleteHistos(liter->second);
  }
  HistMapTime_.clear();

  liter = HistMapZ_.begin();
  for (; liter !=HistMapZ_.end();++liter){
    deleteHistos(liter->second);
  }
  HistMapZ_.clear();

  hexaGeom.clear();
  squareGeom.clear();
}



void HGCSSGeometryConversion::initialiseSquareMap(const double xymin, const double side){
  initialiseSquareMap(squareMap(),xymin,side,true);
  fillXY(squareMap(),squareGeom);
}

void HGCSSGeometryConversion::initialiseSquareMap(TH2Poly *map, const double xymin, const double side, bool print){
  unsigned nx=static_cast<unsigned>(xymin*2./side);
  unsigned ny=nx;
  unsigned i,j;
  Double_t x1,y1,x2,y2;
  Double_t dx=side, dy=side;
  x1 = -1.*xymin;
  x2 = x1+dx;
  
  for (i = 0; i<nx; i++) {
    y1 = -1.*xymin;
    y2 = y1+dy;
    for (j = 0; j<ny; j++) {
      map->AddBin(x1, y1, x2, y2);
      y1 = y2;
      y2 = y1+dy;
    }
    x1 = x2;
    x2 = x1+dx;
  }
  
  if (print) {
    std::cout <<  " -- Initialising squareMap with parameters: " << std::endl
	      << " ---- xymin = " << -1.*xymin << ", side = " << side
	      << ", nx = " << nx << ", ny=" << ny
	      << std::endl;
  }
  
}

void HGCSSGeometryConversion::initialiseHoneyComb(const double xymin, const double side){
  initialiseHoneyComb(hexagonMap(),xymin,side,true);
  fillXY(hexagonMap(),hexaGeom);
}

void HGCSSGeometryConversion::initialiseHoneyComb(TH2Poly *map, const double xymin, const double side, bool print){
  //xstart,ystart,side length,
  double d=sqrt(3.)*side;
  unsigned nx=static_cast<unsigned>(xymin*2./d);
  unsigned ny=static_cast<unsigned>(xymin*4./(3.*side));
  if (print) {
    std::cout << " -- Initialising HoneyComb with parameters: " << std::endl
	      << " ---- xymin = " << -1.*xymin << ", side = " << side
	      << ", nx = " << nx << ", ny=" << ny << std::endl;
  }
  map->Honeycomb(-1.*xymin,-1.*xymin,side,nx,ny);
  
  
}


void HGCSSGeometryConversion::fillXY(TH2Poly* hist, std::map<int,std::pair<double,double> > & geom){
  TIter next(hist->GetBins());
  TObject *obj=0; 
  TH2PolyBin *polyBin = 0;
  geom.clear();
  
  while ((obj=next())){
    polyBin=(TH2PolyBin*)obj;
    int id = polyBin->GetBinNumber();
    std::pair<double,double> xy = std::pair<double,double>((polyBin->GetXMax()+polyBin->GetXMin())/2.,(polyBin->GetYMax()+polyBin->GetYMin())/2.);
    geom.insert(std::pair<unsigned,std::pair<double,double> >(id,xy));
  }
  
  std::cout << " -- Check geomMap: size = " << geom.size() << std::endl;
  //std::map<int,std::pair<double,double> >::iterator liter=geom.begin();
  //for ( ; liter != geom.end();++liter){
  //std::cout << " id " << liter->first << ": x=" << liter->second.first << ", y=" << liter->second.second << std::endl;
  //}
  
}

void HGCSSGeometryConversion::deleteHistos(std::vector<TH2Poly *> & aVec){
  if (aVec.size()!=0){
    for (unsigned iL(0); iL<aVec.size();++iL){
      //if (aVec[iL]) aVec[iL]->Delete();
      delete aVec[iL]; aVec[iL]=0;
    }
    aVec.clear();
  }
}

void HGCSSGeometryConversion::setGranularity(const std::vector<unsigned> & granul){
  granularity_.reserve(granul.size());
  for (unsigned iL(0); iL<granul.size();++iL){
    granularity_.push_back(granul[iL]);
  }
}

double HGCSSGeometryConversion::cellSize(const unsigned aLayer, const double aR) const{
  if (theDetector().subDetectorByLayer(aLayer).isScint) return 10.*granularity_[aLayer];
  return cellSize_;
  //double r1 = theDetector().subDetectorByLayer(aLayer).radiusLim;
  //if (aR<r1) return cellSize_*3;
  //else return cellSize_*4;
}

double HGCSSGeometryConversion::cellSizeInCm(const unsigned aLayer, const double aR) const{
  return cellSize(aLayer, aR)/10.;
}

void HGCSSGeometryConversion::initialiseHistos(const bool recreate,
					       std::string uniqStr,
					       const bool print){

   for (unsigned iS(0); iS<theDetector().nSections();++iS){
     resetVector(HistMapE_[theDetector().detType(iS)],"EmipHits"+uniqStr,theDetector().detName(iS),theDetector().subDetectorBySection(iS),theDetector().nLayers(iS),recreate,print);
     //std::cout << " check: " << HistMapE_[theDetector().detType(iS)].size() << std::endl;
     
     std::vector<double> avgvecE;
     avgvecE.resize(theDetector().nLayers(iS),0);
     avgMapE_[theDetector().detType(iS)]=avgvecE;
     
     resetVector(HistMapTime_[theDetector().detType(iS)],"TimeHits"+uniqStr,theDetector().detName(iS),theDetector().subDetectorBySection(iS),theDetector().nLayers(iS),recreate,print);
     //std::cout << " check: " << HistMapTime_[theDetector().detType(iS)].size() << std::endl;

     resetVector(HistMapZ_[theDetector().detType(iS)],"zHits"+uniqStr,theDetector().detName(iS),theDetector().subDetectorBySection(iS),theDetector().nLayers(iS),recreate,print);
     //std::cout << " check: " << HistMapZ_[theDetector().detType(iS)].size() << std::endl;
     
     std::vector<double> avgvecZ;
     avgvecZ.resize(theDetector().nLayers(iS),0);
     avgMapZ_[theDetector().detType(iS)]=avgvecZ;
   }
 }

void HGCSSGeometryConversion::fill(const DetectorEnum type,
				   const unsigned newlayer,
				   const double & weightedE,
				   const double & aTime,
				   const double & posx,
				   const double & posy,
				   const double & posz)
{

  HistMapE_[type][newlayer]->Fill(posx,posy,weightedE);
  HistMapTime_[type][newlayer]->Fill(posx,posy,weightedE*aTime);
  HistMapZ_[type][newlayer]->Fill(posx,posy,weightedE*posz);
  avgMapZ_[type][newlayer] += weightedE*posz;
  avgMapE_[type][newlayer] += weightedE;
}

double HGCSSGeometryConversion::getAverageZ(const unsigned layer){
  const HGCSSSubDetector & subdet = theDetector().subDetectorByLayer(layer);
  unsigned newlayer = layer-subdet.layerIdMin;
  double avg = 0;
  if (avgMapE_[subdet.type][newlayer]>0)
    avg =avgMapZ_[subdet.type][newlayer]/avgMapE_[subdet.type][newlayer];
  return avg;
}

TH2Poly * HGCSSGeometryConversion::get2DHist(const unsigned layer,std::string name){
  const HGCSSSubDetector & subdet = theDetector().subDetectorByLayer(layer);
  unsigned newlayer = layer-subdet.layerIdMin;
  if (name == "E") return HistMapE_[subdet.type][newlayer];
  else if (name == "Time") return HistMapTime_[subdet.type][newlayer];
  else if (name == "Z") return HistMapZ_[subdet.type][newlayer];
  else {
    std::cerr << " ERROR !! Unknown histogram name. Exiting..." << std::endl;
    exit(1);
  }
}

unsigned HGCSSGeometryConversion::getGranularity(const unsigned aLayer, const HGCSSSubDetector & adet){
  unsigned idx = adet.layerIdMin+aLayer;
  return granularity_[idx];
}

void HGCSSGeometryConversion::resetVector(std::vector<TH2Poly *> & aVec,
					  std::string aVar,
					  std::string aString,
					  const HGCSSSubDetector & aDet,
					  const unsigned nLayers,
					  bool recreate, 
					  bool print)
{
  //std::cout << " vector size: " << aVar << " " << aString << " = " << aVec.size() << std::endl;
  if (recreate){
    for (unsigned iL(0); iL<aVec.size();++iL){
      aVec[iL]->Delete();
    }
    aVec.clear();
  }
  if (aVec.size()!=0){
    for (unsigned iL(0); iL<aVec.size();++iL){
      aVec[iL]->Reset("");
    }
  }
  else {
    if (nLayers > 0){
      aVec.resize(nLayers,0);
      if (print) std::cout << " -- Creating " << nLayers << " 2D histograms for " << aVar << " " << aString 
	//<< " with " << nBins << " bins between " << min << " and " << max 
		<< std::endl;
      for (unsigned iL(0); iL<nLayers;++iL){
	std::ostringstream lname;
	lname << aVar << "_" << aString << "_" << iL ;
	aVec[iL] = new TH2Poly();
	aVec[iL]->SetName(lname.str().c_str());
	if (print && aVar.find("EmipHits")!=aVar.npos) std::cout << " ---- Layer " << iL;
	if (aDet.isScint){
	  double newcellsize = 10.*getGranularity(iL,aDet);
	  initialiseSquareMap(aVec[iL],width_,newcellsize,print && aVar.find("EmipHits")!=aVar.npos);

	}
	else {
	  initialiseHoneyComb(aVec[iL],width_,cellSize_,print && aVar.find("EmipHits")!=aVar.npos);
	}

      }
    }
  }
  //std::cout << " vector size after: " << aVar << " " << aString << " = " << aVec.size() << std::endl;
}





