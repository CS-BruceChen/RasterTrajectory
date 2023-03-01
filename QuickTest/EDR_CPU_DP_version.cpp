//需要调试，最后给出的EDR结果不对

#include<iostream>
#include<vector>
#include<omp.h>

#define print(a) (std::cout<<(a)<<std::endl)
#define getFirstPoint(T) ((*(T.points))[T.startIndex])


const float epsl = 1.0;//thresh hold

//test_ok
struct Point{
	float x;
	float y;
	Point(float u,float v):x(u), y(v){};
};


struct SEQ {
	std::vector<Point>* points;
	unsigned len;
	unsigned startIndex;
	unsigned pointNum;
	SEQ(std::vector<Point>* data):points(data),len(data->size()),startIndex(0),pointNum(data->size()){}
	SEQ(SEQ& T):points(T.points),len(T.len),startIndex(T.startIndex),pointNum(T.pointNum){}
};

SEQ head(SEQ T){
	SEQ headT(T);
	headT.startIndex++;
	headT.pointNum--;
	return headT;
}


float ABS(float x){
	return x>0?x:-x;
}

unsigned distance(unsigned x,unsigned y){
	if(x>y) return x-y;
	else return y-x;
}

//test_ok
unsigned subcost(SEQ T,SEQ S) {
	Point fp_T = getFirstPoint(T);
	Point fp_S = getFirstPoint(S);
	if(ABS(fp_T.x-fp_S.x) <= epsl && ABS(fp_T.y-fp_S.y) <= epsl) return 0;
	else return 1;
}

//test_ok
unsigned min(unsigned a,unsigned b,unsigned c){
	if(a <= b && a <= c) return a;
	else if(b <= a && b <= c) return b;
	else if(c <= a && c <= b) return c;
	else return 0;//default
}

unsigned getEDR(SEQ T,SEQ S) {
	if(T.pointNum==0){
		return S.pointNum;
	}
	else if(S.pointNum==0){
		return T.pointNum;
	}
	else{
		return min(
			getEDR(head(T),head(S))+subcost(T,S),
			getEDR(head(T),S)+1,
			getEDR(T,head(S))+1
		);	
	}
}

struct DPInfo{
	unsigned wd;
	unsigned ht;
	bool isWdBiggerThanHt;
	unsigned min_wd_ht;
	unsigned d_w_h;
	unsigned total_step;
	unsigned** DP_Array;
	std::vector<Point>* wdPoints;
	std::vector<Point>* htPoints;
	DPInfo(SEQ T,SEQ S){//还没有析构
		wd = T.len;
		ht = S.len;
		isWdBiggerThanHt = wd > ht ? true : false;
		min_wd_ht = wd < ht ? wd : ht;
		d_w_h = distance(wd, ht);
		total_step = wd + ht;
		//开辟内存
		DP_Array = new unsigned*[wd+1];
		for(unsigned i=0;i<wd+1;++i){
			DP_Array[i]=new unsigned[ht+1];
		}
		//初始化
		for(unsigned i = 1; i <= wd; ++i){
			for(unsigned j = 1; j <= ht; ++j){
				DP_Array[i][j] = 0;
			}
		}
		for(unsigned i = 0; i <= wd; ++i){
			DP_Array[i][0] = i;
		}
		for(unsigned j = 0; j <= ht; ++j){
			DP_Array[0][j] = j;
		}
		wdPoints = T.points;
		htPoints = S.points;
	};
	void showDPInfo(){
		print(wd);
		print(ht);
		print(isWdBiggerThanHt);
		print(min_wd_ht);
		print(d_w_h);
		print(total_step);
	}
	void showDPArray(){
		print("DPArray---------------------------------");
		for(unsigned j = 0; j <= ht; j++){
			for(unsigned i = 0; i <= wd; i++){
				std::cout<<DP_Array[i][j]<<" ";
			}
			std::cout<<std::endl;
		}
		print("DPArray---------------------------------");
	}
};

bool isInWorkingRange(unsigned tid,unsigned step_now,DPInfo* dpinfo){
	unsigned arrayId = tid+1;//tid in [0,min_wd_ht-1]
	unsigned min_bound = 1;
	unsigned max_bound = dpinfo->min_wd_ht;
	if(step_now<=dpinfo->min_wd_ht){
		min_bound = 1;
		max_bound = step_now;
	}
	else if(step_now<=dpinfo->min_wd_ht+dpinfo->d_w_h){
		min_bound = 1;
		max_bound = dpinfo->min_wd_ht;
	}
	else if(step_now<=dpinfo->total_step){
		min_bound = 1 + step_now - dpinfo->min_wd_ht - dpinfo->d_w_h;
		max_bound = dpinfo->min_wd_ht;
	}
	
	if(min_bound <= arrayId && arrayId <= max_bound) return true;
	else return false;
}

unsigned subcost_GPU(std::vector<Point>* wdPoints,unsigned wdIndex,std::vector<Point>* htPoints,unsigned htIndex){
	Point fp_T = (*wdPoints)[wdIndex-1];//index in [1,wd] 因此应当减一，才能取到正确的
	Point fp_S = (*htPoints)[htIndex-1];
	unsigned subcost_result = 0;
	if(ABS(fp_T.x-fp_S.x) <= epsl && ABS(fp_T.y-fp_S.y) <= epsl) subcost_result = 0;
	else subcost_result = 1;
	std::cout<<"subcost for ( "<<wdIndex<<", "<<htIndex<<" ) is:"<<subcost_result<<std::endl;
	return subcost_result;
}

void slashDP(DPInfo* dpinfo,unsigned step_now){
#pragma omp parallel
{
	unsigned tid = omp_get_thread_num();
	bool isTidInRange = isInWorkingRange(tid,step_now,dpinfo);
	if(isTidInRange) {
		unsigned x = dpinfo->isWdBiggerThanHt ? (step_now - tid) : (1 + tid);
		unsigned y = dpinfo->isWdBiggerThanHt ? (1 + tid) : (step_now - tid);
		dpinfo->DP_Array[x][y] = min(
			dpinfo->DP_Array[x-1][y] + 1, 
			dpinfo->DP_Array[x][y-1] + 1, 
			dpinfo->DP_Array[x-1][y-1] + subcost_GPU(dpinfo->wdPoints, x, dpinfo->htPoints, y)
		);	
	}
}
}

unsigned getEDR_GPU(SEQ T,SEQ S){
	DPInfo dpinfo(T,S);
//	dpinfo.showDPInfo();
	omp_set_num_threads(dpinfo.min_wd_ht);
	
	for(unsigned i = 1; i <= dpinfo.total_step; i++){
		std::cout<<"loop:"<<i<<"---------------------"<<std::endl;
		slashDP(&dpinfo,i);
		dpinfo.showDPArray();
	}	
	return dpinfo.DP_Array[dpinfo.wd][dpinfo.ht];
}

int main(){
	std::vector<Point> t,s;
	t.push_back(Point(0,0));
	t.push_back(Point(2,0));
	t.push_back(Point(0,2));
	
	s.push_back(Point(0,0));
	s.push_back(Point(0,0));
	s.push_back(Point(0,0));
	s.push_back(Point(3,0));
	s.push_back(Point(0,0));
	SEQ T(&t);
	SEQ S(&s);
	
	std::cout<<"the EDR beteen T and S is:"<<getEDR(T,S)<<std::endl;
//	std::cout<<"the EDR beteen T and S is:"<<getEDR_GPU(T,S)<<std::endl;
//	print(getEDR(T,S));
//	print(getEDR_GPU(S,T));
	getEDR_GPU(S,T);
	
	return 0;
}
