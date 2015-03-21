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



int json_path(json_t* root, char* path, json_t* node){
	char **children;
	int layers=json_is_valid_path(path,&children);
	json_t *tmp=NULL,*tmp2;
	int ret=0;
	int i=0;
	printf("setting/getting\n");

	if(layers>0){
		tmp=root;
		for(i=layers-1;i>=0;i--){
			printf("processing layers %i: %s\n",i,children[i]);
			int key;
			if(children[i][0]=='.'){
				//normal object
				printf("object!\n");
				tmp2=json_object_get(tmp, children[i]+1);
				if(tmp2==NULL){
					if(node!=NULL){
						//if in middle of path
						//TODO: check correct indizes of following 
					
						if(i==0 || 1){
							if(i==0){
								tmp2=node;
							}else{
								tmp2=json_object();
							}
							printf("INSERT: %i\n",	json_object_set(tmp, children[i]+1,tmp2));
							tmp=json_object_get(tmp, children[i]+1);
							printf("Result: `%s`\n",json_string_value(tmp));
							printf("runs!!!\n");
						}else{
							return 1;
						}
						
					}else{
						tmp=NULL;
						return 1;
					}
				}else{
					tmp=tmp2;
				}
			}else if(sscanf(children[i],"[%i]",&key)==1){
				printf("ARRAY! %i\n",key);
// 				if(json_is_array(tmp) /*&& key<json_array_size(tmp)*/){
				tmp2=json_array_get(tmp,key);
			}else{
				printf("invalid path!\n");
				return -1;
			}
			
			if(tmp==NULL){
				
				
				
				printf("unknown child\n");
				ret=1;
				break;
			}
			
		}
		for(i=0;i<layers;i++){
 			free(children[i]);
		}
	}else{
		printf("invalid path\n");
		return -1;
	}
	
	//free up memory

  	free(children);
	printf("Result: `%s`\n",json_string_value(tmp));
	node=tmp;

	return ret;
}

json_t* json_path_get(json_t* root, char* path){
	json_t *ret=NULL; 
	if(0==json_path(root,path,ret)){
		return ret;
	}else{
		return NULL;
	}
}

int json_path_set(json_t* root, char* path, json_t* node){
	printf("Result: `%s`\n",json_string_value(node));
	return json_path(root,path,node);
}

int main(){
//  	char p[] = "$.c0:4a:00:ed:f1:bc.network[1][3].addresses[1]";
 	char p[] = "$.c0:4a:00:ed:f1:bc.netwo.rk.mac";
// 	printf("valid: %i\n", json_is_valid_path(p));
	json_t *tst_json = json_load_file("json3.json", 0, NULL);
	char **children;
	int layers=json_is_valid_path(p,&children);
	json_t* tst=json_string("ASDASDADS");
// 	json_string_set(tst, );
	json_path_set(tst_json,p,tst);
// 	json_object_set(json_object_get(tst_json, "c0:4a:00:ed:f1:bc"),"netw",tst);
	printf("%s",json_dumps(tst_json,JSON_INDENT(3)));
	return 0;
}