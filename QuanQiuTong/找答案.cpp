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

int find_similar(int start,int len,int type=1,int cnt=-1) // 在ref中找到与query(start,start+len)距离最小的子串
{
	int ans=len,ans_index=0,ans_len=0,p;
	for(int lenn=len;lenn<=len;lenn+=20)
		for(int i=0;i<ref_len-len;i+=3000)
	    {
	    	p=align2(ref,query,i,start,lenn,len);
	    	if(ans>p)
	    	{
	    		ans=p;
	    		ans_index=i;
	    		ans_len=lenn;
			}
		}
	printf("start=%d len=%d 答案：距离=%d，位置=%d，len-距离=%d，占比=%d\n",start,len,ans,ans_index,len-ans,ans*1000/len);
	
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
    ref = load("chr6_substring.fasta");
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
	
	
	dg(0,query.size(),1);
    sort(anss+1,anss+cnt+1);
    for(int i=1;i<=cnt;i++)
    {
    	printf("%d ",anss[i]);
	}
	
	
	//const int N=15;
	//vector<int> ans={0,1811,2103,3607,4205,9301,9603,9915,10203,10505,14705,15607,19209,21617};
	//vector<int> ans={0,2103,9301,9603,9915,10203,10505,14705,15607,19209,21617};
	//vector<int> ans={1811,9301};
	//vector<int> ans={0, 1800,  9000, 9300, 9600, 9900, 10200, 10500, 10800, 14700, 21600};
	//static int ans[N]={0,151,6953,7305,9201,9259,9301,9353,9403,9507,9553,9605,9657,9709,9753,9857,9901,9953,10003,10057,10105,10157,10203,10307,10355,10457,10509,10557,14259,14461,14557,14709,15559,15611,19213};
	//int ans[N]={0,2103,3607,4205,6953,7305,9201,10561,14259,14461,14557,14709,15559,16365,19213,20665,21617};
	//int ans[N]={0,1811,2103,3607,4205,9301,9603,9915,10203,10505,14705,15607,19209,21617};
	//vector<int> ans={0,1811,2103,3607,4205,9135, 9557, 9979, 10401, 10743,14705,15607,19209,21617};
	/*int N=ans.size();
	
	
	for(int i=0;i<=N-2;i++)
	{
		output[i][0]=ans[i],output[i][1]=ans[i+1];
		find_similar(output[i][0],output[i][1]-output[i][0],0,i);
		//output[i][2]=find_similar(output[i][0],output[i][1]-output[i][0]+1,0);
		//output[i][3]=output[i][2]+output[i][1]-output[i][0];
	}
	for(int i=0;i<=N-2;i++)
	{	
		printf("(%d,%d,%d,%d),",output[i][0],output[i][1],output[i][2],output[i][3]);
	}
	printf("最终结果=%d\n",tot);*/

    return 0;
}
