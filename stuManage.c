#include <stdio.h>
#include <stdlib.h>
#define YELLOW "\033[33m"
#define RESET "\033[0m"
#define GREEN "\033[32m"
typedef struct Student{

	int id;
	char name[10];
	int score;
}student;
student stu[100];
int count=0;

void menu(){
	printf(YELLOW"======================\n");
	printf("欢迎使用学生管理系统\n");
	printf("======================\n");
	printf("1.显示所有学生\n2.添加学生\n3.删除学生\n4.修改学生信息\n5.查询学生\n6.退出\n");
	printf("======================\n"RESET);


}
void show(){
		if(count==0){
			printf("暂无数据！\n");	
				return;		
			}else{
		 	printf("ID\t姓名\t分数\n");
			for(int i=0;i<count;i++){
			
			printf("%d\t%s\t%d\n",stu[i].id,stu[i].name,stu[i].score);
			
			}
			}
}
void loadData(){
	FILE *fp=fopen("student.txt","r+");
	if(!fp){
		count=0;
		return;
	}
	
	while(fread(&stu[count],1,sizeof(student),fp)>0){
		count++;
	}
	fclose(fp);
}
void saveData(){
	FILE *fp=fopen("student.txt","r+");
	if(!fp){
		printf("保存失败！\n");
		return;
	}
	for(int i=0;i<count;i++){
		fwrite(&stu[i],1,sizeof(student),fp);
	}
	fclose(fp);
	}
void add(){
			student s;
			printf("请输入ID：");
			scanf("%d",&s.id);
			printf("请输入姓名：");
			scanf("%s",s.name);
			printf("请输入分数：");
			scanf("%d",&s.score);
			stu[count++]=s;
			printf("添加完成！\n");
}
void delete(){

	if(count==0){
		printf("暂无学生数据！");
		return;
	}
	int id,i;
	int flag=0;
	printf("请输入要删除的学生id:");
	scanf("%d",&id);
	for(i=0;i<count;i++){
		if(stu[i].id==id){
			flag=1;
			break;
		}
	}
	if(!flag){
		printf("未找到该学生～\n");
		return;
	}
	for(i=id;i<count-1;i++){
		stu[i]=stu[i+1];
		
	}
	count--;
	printf("删除成功！\n");
}
void modify(){
	if(count==0){
		printf("暂无学生数据!\n");
		return;
	}
	int id,i;
	int flag=0;
	printf("请输入要修改的学生的id: ");
	scanf("%d",&id);
	for(i=0;i<count;i++){
		if(stu[i].id==id){
			flag=1;
			break;
		}
	}
	if(flag==0){
		printf("未找到该学生～\n");
		return;
	}
	printf("请输入新姓名：");
	scanf("%s",stu[i].name);
	printf("请输入新分数： ");
	scanf("%d",&stu[i].score);
	printf("修改完成！\n");	
}
void search(){
	if(count==0){
		printf("暂无学生数据！");
		return;
	}
	int id,i;
	int flag=0;
	printf("请输入要查询的学生的id:");
	scanf("%d",&id);
	for(i=0;i<count;i++){
		if(stu[i].id==id){
			flag=1;
			printf("ID\t姓名\t分数\n");
						
			printf("%d\t%s\t%d\n",stu[i].id,stu[i].name,stu[i].score);
			break;
			}
	}
	if(flag==0){
		printf("未找到该学生～\n");
	}
}
int main(){
	loadData();
	while(1){
	menu();
	int n=0;
	printf("请选择（1-6）：");
	scanf("%d",&n);

	switch(n){

		case 1://显示
			show();
			break;
		case 2://添加
			
			add();
			break;
		case 3://删除
			delete();
			break;

		case 4://修改
			modify();
			break;
		case 5://查询
			search();
			break;
		case 6://退出
			saveData();
			printf("成功退出!\n");
			return 0;
	}
	}

	printf("\n");
	return 0;
}
