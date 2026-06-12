#include <stdio.h>
#include <stdlib.h>
#include <mysql/mysql.h>
#define YELLOW "\033[33m"
#define RESET "\033[0m"
#define GREEN "\033[32m"
void menu(){
	printf(YELLOW"======================\n");
	printf("欢迎使用学生管理系统\n");
	printf("======================\n");
	printf("1.显示所有学生\n2.添加学生\n3.删除学生\n4.修改学生信息\n5.查询学生\n6.退出\n");
	printf("======================\n"RESET);


}

/*void mysqlConnect(){
	MYSQL *conn;
	conn=mysql_init(NULL);
	
	mysql_real_connect(conn,"192.168.110.182","root","123456","qrs2",3306,NULL,0);
	
	char sql[100];
	sprintf(sql,"select * from s1");
	mysql_query(conn,sql);
	
	MYSQL_RES *res=mysql_store_result(conn);
	
	MYSQL_ROW row;
	while(row=mysql_fetch_row(res)){
		printf("%s\t%s\t%s\t%s\n",row[0],row[1],row[2],row[3]);
	}
	mysql_free_result(res);
	mysql_close(conn);

}*/
MYSQL* connect(){
	MYSQL *conn=mysql_init(NULL);
	if(conn==NULL){
		printf("init error");
		return NULL;
	}
	mysql_real_connect(conn,"192.168.110.182","root","123456","qrs2",3306,NULL,0);
		
	return conn;
	
}
void show(){
	MYSQL *conn=connect();
	char sql[100];
	sprintf(sql,"select * from s1");
	mysql_query(conn,sql);
	
	MYSQL_RES *res=mysql_store_result(conn);
	
	MYSQL_ROW row;
	while(row=mysql_fetch_row(res)){
		printf("%s\t%s\t%s\t%s\n",row[0],row[1],row[2],row[3]);
	}
	mysql_free_result(res);
	mysql_close(conn);
}



int main(){
	while(1){
	menu();
	int n=0;
	printf("请选择（1-6）：");
	scanf("%d",&n);

	switch(n){

		case 1://显示
			
			show();
			break;
		//case 2://添加
			
			//break;
		//case 3://删除
		
	//		break;

		//case 4://修改
	
	//		break;
	//	case 5://查询
	//		break;
		case 6://退出
			printf("成功退出!\n");
			return 0;
	}
	}

	printf("\n");
	return 0;
}
