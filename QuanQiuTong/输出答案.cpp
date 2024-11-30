#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <unordered_map>
#include "../../../edlib.cpp"// 使用 edlib 库

// 计算 DNA 序列的反向互补序列
std::string rc(const std::string& dna_seq) {
    // 定义碱基的互补关系
    std::unordered_map<char, char> complement = {
        {'A', 'T'}, {'T', 'A'},
        {'C', 'G'}, {'G', 'C'},
        {'a', 't'}, {'t', 'a'},
        {'c', 'g'}, {'g', 'c'}
    };

    std::string reverse_complement;
    reverse_complement.reserve(dna_seq.size());

    // 反转序列并生成互补碱基
    for (auto it = dna_seq.rbegin(); it != dna_seq.rend(); ++it) {
        reverse_complement.push_back(complement.count(*it) ? complement[*it] : *it);
    }

    return reverse_complement;
}

// 使用 edlib 计算两个序列之间的编辑距离
int align(const std::string& seq1, const std::string& seq2) {
    EdlibAlignResult result = edlibAlign(
        seq1.c_str(), seq1.size(),
        seq2.c_str(), seq2.size(),
        edlibDefaultAlignConfig()
    );

    int edit_distance = result.editDistance;
    edlibFreeAlignResult(result);  // 释放内存
    return edit_distance;
}

// 使用 edlib 计算两个序列之间的编辑距离
int align2(const std::string& seq1, const std::string& seq2, int s1,int s2,int len1,int len2) {
    EdlibAlignResult result = edlibAlign(
        seq1.c_str()+s1, len1,
        seq2.c_str()+s2, len2,
        edlibDefaultAlignConfig()
    );

    int edit_distance = result.editDistance;
    edlibFreeAlignResult(result);  // 释放内存
    return edit_distance;
}

// 计算目标值
int calculate_value(const std::string& ref, const std::string& query,
                    int ref_st, int ref_en, int query_st, int query_en) 
{
	static int cnt=0;
	cnt++;
	if(cnt%1000==0)
		printf("%d ",cnt);
	
    std::string A = ref.substr(0, ref_st);
    std::string a = query.substr(0, query_st);
    std::string B = ref.substr(ref_st, ref_en - ref_st);
    std::string b_rc = rc(query.substr(query_st, query_en - query_st));
    std::string C = ref.substr(ref_en);
    std::string c = query.substr(query_en);

    // 计算并返回最终的结果
    return std::max(0, static_cast<int>(ref.size())
                       - align(A, a)
                       - align(B, b_rc)
                       - align(C, c));
}

// 从文件中加载序列内容
std::string load(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "无法打开文件: " << filename << std::endl;
        exit(1);  // 退出程序
    }

    std::string sequence;
    std::getline(file, sequence, '\0');  // 读取整个文件内容
    sequence.erase(sequence.find_last_not_of(" \n\r\t") + 1);  // 去除尾部空白字符
    return sequence;
}

using namespace std;

string ref,query;

int tot=0;
int ref_len;
int output[10000][4];

FILE* fp;

int find_similar(int start,int len,int type=1,int cnt=-1) // 在ref中找到与query(start,start+len)距离最小的子串
{
	/*int ans=len,ans_index=0,ans_len=0,p;
	int record_len = len, record_index = -1;
	for(int step=50;step>=20;step/=8) // 逐渐缩小步长
	{
		for(int lenn=record_len-step/2;lenn<=record_len+step/2;lenn+=step/4)  // 枚举在ref上的长度
		{
			int l,r;
			if(record_index<0)
				l=0, r=ref_len-lenn;
			else
				l=max(0,record_index-step*10), r=min(ref_len-lenn,record_index+step*10);
				
			for(int i=l;i<r;i+=step)  // 枚举在ref上的位置
		    {
		    	p=align2(ref,query,i,start,lenn,len);
		    	//printf("%d ",p*1000/len);
		    	if(lenn==record_len)
					printf("[%d,%d] ",i,p*1000/len);
		    	if(ans>p)
		    	{
		    		ans=p;
		    		ans_index=i;
		    		ans_len=lenn;
				}
			}
		}
		printf("\n");
		record_index = ans_index;
		record_len = ans_len;
	}*/
	
	int len_range[9]={-400,-200,-50,-20,0,20,50,200,400};
	int ans=len,ans_index=0,ans_len=0,p;
	for(int lenn=len-600;lenn<=len+600;lenn+=40)
	{
	/*for(int len_i=0,lenn;len_i<9;len_i++)
	{
		lenn=len+len_range[len_i];*/
		for(int i=0;i<ref_len-len;i+=10)
	    {
	    	p=align2(ref,query,i,start,lenn,len);
	    	if(ans>p)
	    	{
	    		ans=p;
	    		ans_index=i;
	    		ans_len=lenn;
			}
		}
		//printf("#");
	}
		
	printf("start=%d len=%d 答案：距离=%d，位置=%d~%d %d，len-距离=%d，占比=%d\n",start,len,ans,ans_index,ans_index+ans_len,ans_len,len-ans,ans*1000/len);
	fprintf(fp,"start=%d len=%d 答案：距离=%d，位置=%d~%d %d，len-距离=%d，占比=%d\n",start,len,ans,ans_index,ans_index+ans_len,ans_len,len-ans,ans*1000/len);
	
	if(cnt>=0)
	{
		output[cnt][2]=ans_index;
		output[cnt][3]=ans_index+ans_len;
	}
	
	if(!type)
		tot+=len-ans-30;
	
	if(type)
		return ans;
	else
		return ans_index;
}

int anss[1000000],cnt=0;
const int step=3000;

int dg(int l,int r,int deep)
{
	if(deep==5)return 0;
	int ans=0,ansindex=l;
	/*if(deep==1)
	{
		ans=-1;
		ansindex=9901;
	}
	else if(deep==2)
	{
		if(r<10000)
		{
			ansindex=9301;
			ans=-2;
		}
		else
		{
			ansindex=10203;
			ans=-2;
		}
	}
	else*/
	{
		if(r-l<step*2) // 范围太小，没必要再枚举了
		{
			return 0;
		}
		ans=r-l+1 - find_similar(l,r-l);ansindex=-1;
		for(int i=l+1;i<r-1;i+=step)
		{
			int p=(r-l+1)-(find_similar(l,i-l+1)+find_similar(i+1,(r-1)-(i+1)+1))-30;
			//printf("#%d %d\n",i,p);
			if(p>ans)
			{
				ans=p;
				ansindex=i;
			}
		}
	}
	if(ansindex<0) // 拆了后得分更低
	{
		return 0;
	}
	printf("%d ans=%d index=%d\n",deep,ans,ansindex);
	ans=dg(l,ansindex,deep+1)+dg(ansindex+1,r,deep+1);
	anss[++cnt]=ansindex;
	return ans;
}

int main() {
    // 从文件中加载序列
    ref = load("chr6_substring_2.fasta");
    query = load("../query.txt");
    //ref = load("reference2.txt");
    //query = load("query2.txt");
    
    ref_len=ref.size();

	string A ;
    string a ;
    
    //find_similar(6000,4000);
    
    /*
    for(int i=3000;i<=5000;i+=200)
    {
    	for(int j=1000;j<=6000;j+=400)
	    {
	    	find_similar(i,j);
		}
		printf("\n");
	}
	*/
	
	
	/*dg(0,query.size(),1);
    sort(anss+1,anss+cnt+1);
    for(int i=1;i<=cnt;i++)
    {
    	printf("%d ",anss[i]);
	}*/
	
	
	//const int N=15;
	//vector<int> ans={0,145064};
	//vector<int> ans={0,12250,22500,39001,51001,66003,82944,93001,103000,110000,119032,130000,145064};
	//vector<int> ans={0,10000,20000,30000,40000,50000,60000,70000,80000,90000,100000,110000,120000,130000,140000};
	vector<int> ans={0,1000,2000,3000,4000,5000,6000,7000,8000,9000,10000,11000,12000,13000,14000,15000,16000,17000,18000,19000,20000,21000,22000,23000,24000,25000,26000,27000,28000,29000,30000,31000,32000,33000,34000,35000,36000,37000,38000,39000,40000,41000,42000,43000,44000,45000,46000,47000,48000,49000,50000,51000,52000,53000,54000,55000,56000,57000,58000,59000,60000,61000,62000,63000,64000,65000,66000,67000,68000,69000,70000,71000,72000,73000,74000,75000,76000,77000,78000,79000,80000,81000,82000,83000,84000,85000,86000,87000,88000,89000,90000,91000,92000,93000,94000,95000,96000,97000,98000,99000,100000,101000,102000,103000,104000,105000,106000,107000,108000,109000,110000,111000,112000,113000,114000,115000,116000,117000,118000,119000,120000,121000,122000,123000,124000,125000,126000,127000,128000,129000,130000,131000,132000,133000,134000,135000,136000,137000,138000,139000,140000,141000,142000,143000,144000,145000,145064};
	int N=ans.size();
	
	fp = fopen("ans.txt", "w");
	
	//fprintf(fp,"%d\n",1);
	
	for(int i=0;i<=N-2;i++)
	{
		output[i][0]=ans[i],output[i][1]=ans[i+1];
		find_similar(output[i][0],output[i][1]-output[i][0],0,i);
	}
	for(int i=0;i<=N-2;i++)
	{	
		printf("(%d,%d,%d,%d),",output[i][0],output[i][1],160600000+output[i][2],160600000+output[i][3]);
		fprintf(fp,"(%d,%d,%d,%d),",output[i][0],output[i][1],160600000+output[i][2],160600000+output[i][3]);
	}
	printf("最终结果=%d\n",tot);
	fprintf(fp,"最终结果=%d\n",tot);
	
	fclose(fp);
    return 0;
}
