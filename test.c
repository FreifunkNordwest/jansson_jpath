#include <jansson.h>
#include <string.h>
#include <regex.h>

int json_is_valid_path(char* source, char*** children){
	char regexString[]="^\\$(\\.([a-z0-9:\\-_]+)|(\\[[0-9]+\\]))+$";
	int buffer_length=5;int maxGroups=4;
	regex_t regexCompiled;
	
	unsigned int m;
	char * cursor;
 
	if (regcomp(&regexCompiled, regexString, REG_EXTENDED)){
		printf("Could not compile regular expression.\n");
		return -1;
	};
 
	m = 0;
	
	*children=malloc(buffer_length*sizeof(char*));
	
	cursor = malloc(strlen(source)+1);
	
	strcpy(cursor,source);
	
	for (m = 0; ; m++){
		regmatch_t groupArray[maxGroups];
		
		if (regexec(&regexCompiled, cursor, maxGroups, groupArray, 0)){
			if(m==0){
				free(cursor);
 				free(*children);
				printf("no match!");
				return 0;
			}
			printf("%i children to be returned\n",m);
			break;  // No more matches
		}
		if(buffer_length<=m){
			buffer_length=2*buffer_length;
			*children=realloc(*children,buffer_length*sizeof(char*));
		}

		int match_length=groupArray[1].rm_eo-groupArray[1].rm_so;

		
// 		printf("unnused: %i\n",groupArray[4].rm_eo);

		
		//store children-identifier
		(*children)[m]=malloc((match_length+1)*sizeof(char));
		strncpy((*children)[m], cursor+groupArray[1].rm_so,match_length);
		(*children)[m][match_length]=0;
		//cut path
		cursor[groupArray[1].rm_so]='\0';
		
		printf("layer %u: [%2u-%2u]: %s\n",m, groupArray[0].rm_so, groupArray[0].rm_eo,(*children)[m]);	
	}
  	free(cursor);
 	regfree(&regexCompiled); 
	return m;
}

int indices_zero_from(char** children, int index){
	int i;
	for(i=index-1;i>=0;i--){
		int key;
		if(sscanf(children[i],"[%i]",&key)==1){
			if(key>0){
				return 0;
			}
		}
	}
	return 1;
}



int json_path(json_t* root, char* path, json_t** node){
	char **children;
	int layers=json_is_valid_path(path,&children);
	json_t *tmp=NULL,*tmp2;
	int ret=0;
	int i=0;
	printf("setting/getting\n");

	if(layers>0){
		tmp=root;
		for(i=layers-1;i>=0;i--){
			printf("------\nprocessing layers %i: %s\n",layers-i,children[i]);
			int key;
			if(children[i][0]=='.'){
				tmp2=json_object_get(tmp, children[i]+1);
			}else if(sscanf(children[i],"[%i]",&key)==1){
				tmp2=json_array_get(tmp,key);
			}else{
				//should never be reached
				printf("invalid path!\n");
				return 1;
			}
// 			printf("------\nprocessing layers %i: %s\n",layers-i,children[i]);
			if(node!=NULL){
					if((json_is_array(tmp2) && children[i-1][0]=='.')||(json_is_object(tmp2) && children[i-1][0]=='[')){
						//if type of node unexpected reset sub-tree
						json_object_del(tmp,children[i]);
						tmp2=NULL;
					}
					
					if(tmp2==NULL || i==0){
						//manipulate/create nodes if (i) nonexistent (ii) wrong type (iii) "final"-node to be inserted
						if(i==0){
							tmp2=*node;
						}else if(i-1<0 || children[i-1][0]=='.'){
							tmp2=json_object();
							printf("obj%i....\n",i);
						}else{
							tmp2=json_array();
							printf("arr....\n");
						}
						
						int r;
						if(children[i][0]=='.'){
							printf("---OBJ-LEN:\n");

							r=json_object_set_new(tmp, children[i]+1,tmp2);
							tmp=json_object_get(tmp, children[i]+1);
						}else{
							if(key==json_array_size(tmp)){
								r=json_array_append_new(tmp,tmp2);
							}else if(key<json_array_size(tmp)){
								r=json_array_set_new(tmp,key,tmp2);
							}else{
								return 1;
							}
							tmp=json_array_get(tmp, key);
							printf("get via key:%i",tmp==NULL);
						}
						printf("INSERT: %s: %i\n",children[i]+1, r);
					}else{
						tmp=tmp2;
					}	
					
			
					
			}else{
// 				printf("unknown child\n");
// 				return 1;
			}

		}
		
		for(i=0;i<layers;i++){
 			free(children[i]);
		}
	}else{
		printf("invalid path\n");
		return 1;
	}
	
	//free up memory

  	free(children);
	
	*node=tmp2;
	printf("Results: `%s`\n",json_string_value(*node));
	return 0;
}

json_t* json_path_get(json_t* root, char* path){
	json_t* ret=NULL; 
	if(0==json_path(root,path,&ret)){
		printf("Results: `%s`\n",json_string_value(ret));
		return ret;
	}else{
		return NULL;
	}
}

int json_path_set(json_t* root, char* path, json_t** node){
	printf("Result: `%s`\n",json_string_value(*node));
	return json_path(root,path,node);
}

int main(){
//  	char p[] = "$.c0:4a:00:ed:f1:bc.network.addresses[1]";
 	char p[] = "$.c0:4a:00:ed:f1:bc.netwo[0][0]";
	char p21[] = "$.c0:4a:00:ed:f1:bc.netwo.as";
// 	printf("valid: %i\n", json_is_valid_path(p));
	json_t *tst_json = json_load_file("json3.json", 0, NULL);
	char **children;
// 	int layers=json_is_valid_path(p,&children);
	json_t* tst=json_string("ASDASDADS");
// 	json_string_set(tst, );
	json_path_set(tst_json,p,&tst);
	tst=json_string("ASDAsDADS");
//  	json_path_set(tst_json,p21,&tst);
// 	printf("VAL: %s\n",json_string_value(json_path_get(tst_json,p)));
// 	json_object_set(json_object_get(tst_json, "c0:4a:00:ed:f1:bc"),"netw",tst);
	printf("%s",json_dumps(tst_json,JSON_INDENT(3)));
	return 0;
}