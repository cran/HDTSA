#include <RcppEigen.h>
#include <Rcpp.h>
#include <iostream>
#include <algorithm>
#include <math.h>
#include <random>
#include <ctime>
#include "testtools.h"

// [[Rcpp::depends(RcppEigen,Rcpp)]]
using namespace std;
using namespace Eigen;
using namespace Rcpp;

Eigen::MatrixXd XiC(int n, int k,int p, int B, double bn, int ken_sign,
                    Eigen::MatrixXd Xi_temp){
  Eigen::MatrixXd kenel = Eigen::MatrixXd::Ones(n-k,n-k);
  if(ken_sign==1){
    for(int i=0; i<n-k; i++){
      for(int j=i+1; j<n-k; j++){
        // if(i!=j){
        //   double temp =double(i-j)/bn;
        //   kenel(i,j) = 25/(12*M_PI*M_PI*temp*temp) * (sin(6*M_PI*temp/5)/(6*M_PI*temp/5) - cos(6*M_PI*temp/5));
        // }
        double temp = double(i - j) / bn;
        double value = 25/(12*M_PI*M_PI*temp*temp) * (sin(6*M_PI*temp/5)/(6*M_PI*temp/5) - cos(6*M_PI*temp/5));
        kenel(i,j) = value;
        kenel(j,i) = value;
      }
    }
  }
  if(ken_sign==2){
    for(int i=0; i<n-k; i++){
      for(int j=0; j<n-k; j++){
        double temp =abs(double(i-j)/bn);
        if(temp<=0.5)
          kenel(i,j) = 1-6*temp*temp+6*temp*temp*temp;
        else if(temp<=1)
          kenel(i,j) = 2*pow((1-temp),3);
        else
          kenel(i,j) = 0;
      }
    }
  }
  if(ken_sign==3){
    for(int i=0; i<n-k; i++){
      for(int j=0; j<n-k; j++){
        double temp =abs(double(i-j)/bn);
        if(temp<=1)
          kenel(i,j) = 1-temp;
        else
          kenel(i,j) = 0;
      }
    }
  }
  // EigenSolver<MatrixXd> eig(kenel);
  Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> eig(kenel);
  Eigen::VectorXd EigenValue =  eig.eigenvalues().real().array();
  for(int i=0;i<n-k;i++){
    if(EigenValue(i)<0)EigenValue(i)=log(double(p))/double(n);
  }
  EigenValue = EigenValue.array().sqrt();
  Eigen::MatrixXd D = EigenValue.asDiagonal();
  Eigen::MatrixXd EigenVector = eig.eigenvectors().real();
  Eigen::MatrixXd Xi = (EigenVector*D*EigenVector.transpose()*Xi_temp.transpose()).transpose();
  return Xi;
}

// [[Rcpp::export]]
double bandwith(Eigen::MatrixXd ft, int k,int p, int d, int ken_type){
  int n = ft.cols();
  int kpd = k*p*d;
  double a_hat,bw=0;
  double atemp1,atemp2;
  Eigen::VectorXd rho = (ft.leftCols(n-1)*ft.rightCols(n-1).transpose()).diagonal().array()/(ft.leftCols(n-1)*ft.leftCols(n-1).transpose()).diagonal().array();
  Eigen::VectorXd sig = ((ft.rightCols(n-1)-rho.asDiagonal()*ft.leftCols(n-1)).array().square().rowwise().sum())/double(n-1);
  //VectorXd bw_list = VectorXd::Zero(kpd*(kpd-1)/2);
  atemp1 = 0.0;
  atemp2 = atemp1;
  if(ken_type==1||ken_type==2){
    for(int i=0;i<kpd;i++){
      atemp1 += 4*rho(i)*rho(i)*pow(sig(i),2)*pow((1-rho(i)),-8);
      atemp2 += pow(sig(i),2)*pow((1-rho(i)),-4);
    }
    a_hat = atemp1/atemp2;
    if(ken_type==1) bw = 1.3221*pow(((n-k)*a_hat),0.2);
    else bw = 2.6614*pow(((n-k)*a_hat),0.2);
  }
  else if(ken_type==3){
    for(int i=0;i<kpd;i++){
      atemp1 += 4*rho(i)*rho(i)*pow(sig(i),2)*pow((1-rho(i)),-6)*pow((1+rho(i)),-2);
      atemp2 += pow(sig(i),2)*pow((1-rho(i)),-4);
    }
    a_hat = atemp1/atemp2;
    bw = 1.1447*pow(((n-k)*a_hat),0.33333333);
  }
  return bw;
}
