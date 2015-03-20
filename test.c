#include <jansson.h>
#include <string.h>
#include <regex.h>

int json_is_valid_path(char* source, char*** children, int** indices){
	char regexString[]="^\\$(\\.([a-z0-9:\\-_]+)(\\[([0-9]+)\\])*)+$";
	int buffer_length=5;int maxGroups=5;
	regex_t regexCompiled;
	
	unsigned int m;
	char * cursor;
 
	if (regcomp(&regexCompiled, regexString, REG_EXTENDED)){
		printf("Could not compile regular expression.\n");
		return -1;
	};
 
	m = 0;
	
	*indices=malloc(buffer_length*sizeof(int));
	*children=malloc(buffer_length*sizeof(char*));
	
	cursor = malloc(strlen(source)+1);
	
	strcpy(cursor,source);
	
	for (m = 0; ; m++){
		regmatch_t groupArray[maxGroups];
		
		if (regexec(&regexCompiled, cursor, maxGroups, groupArray, 0)){
			if(m==0){
// 				free(cursor);
// 				free(children);
// 				free(indices);
				return 0;
			}
			printf("break: %i\n",m);
			break;  // No more matches
		}
		if(buffer_length<=m){
			buffer_length=2*buffer_length;
			*indices=realloc(*indices,buffer_length*sizeof(int));
			*children=realloc(*children,buffer_length*sizeof(char*));
		}

		int match_length=groupArray[2].rm_eo-groupArray[2].rm_so;
		int index_length=groupArray[4].rm_eo-groupArray[4].rm_so;
		

		//if array, store array-index
		if(index_length!=0 && groupArray[4].rm_so>groupArray[2].rm_so){
			printf("unnused+++: %i\n",groupArray[4].rm_eo);
			*indices[m]=(int)strtol(cursor+groupArray[4].rm_so, NULL, 10);
		}else{
			printf("unnused---: %i\n",groupArray[4].rm_eo);
			(*indices)[m]=(-1);
		}
		
		printf("unnused: %i\n",groupArray[4].rm_eo);

		
		//store children-identifier
		(*children)[m]=malloc((match_length+1)*sizeof(char));
		strncpy((*children)[m], cursor+groupArray[2].rm_so,match_length);
		(*children)[m][match_length]=0;
		//cut path
		cursor[groupArray[1].rm_so]='\0';
		
		printf("layer %u: [%2u-%2u]: %s->%i (%i)\n",m, groupArray[0].rm_so, groupArray[0].rm_eo,(*children)[m],(*indices)[m],index_length);	
	}
//  	free(cursor);
 	regfree(&regexCompiled); 
	return m;
}



json_t* json_path_get(json_t* root, char* path){
	char **children;
	int *indices;
	int layers=json_is_valid_path(path,&children,&indices);
	printf("%i Getting %i\n",layers,indices[0]);
	json_t *ret;
	int i=0;
	if(layers>0){
		printf("processing %i layers\n",layers);
		ret=root;
		for(i=layers-1;i>=0;i--){
			printf("processing layers %i: %s\n",i,children[i]);
// 			printf("Getting %i\n",indices[2]);
			ret=json_object_get(ret, children[i]);
			if(ret!=NULL){
				if((json_is_array(ret)!=0) == (indices[i]==(-1)) ){
					printf("invalid array-def\n");
					ret=NULL;
					break;
				}else if(json_is_array(ret) && indices[i]<json_array_size(ret)){
					//valid array-access & index
					printf("valid array\n");
					ret=json_array_get(ret,indices[i]);
				}
			}else{
				printf("invalid child\n");
				ret=NULL;
				break;
			}
			
		}
		for(i=0;i<layers;i++){
			free(children[i]);
		}
	}else{
		printf("invalid path\n");
		ret=NULL;
	}
	
	//free up memory
// 	free(indices);

// 	free(children);
	printf("Result: `%s`\n",json_string_value(ret));
	return ret;
}

json_t* json_path_set(json_t* root, char* path, json_t* node){
	char **children;
	int *indices;
	int *new_created;
	int layers=json_is_valid_path(path,&children,&indices);
	printf("%i Getting %i\n",layers,indices[0]);
	json_t *ret;
	int i=0;
	if(layers>0){
		new_created=malloc(layers*sizeof(int));
		printf("processing %i layers\n",layers);
		ret=root;
		for(i=layers-1;i>=0;i--){
			printf("processing layers %i: %s\n",i,children[i]);
// 			printf("Getting %i\n",indices[2]);
			ret=json_object_get(ret, children[i]);
			if(ret!=NULL){
				if((json_is_array(ret)!=0) == (indices[i]==(-1)) ){
					printf("invalid array-def\n");
					ret=NULL;
					break;
				}else if(json_is_array(ret) && indices[i]<json_array_size(ret)){
					//valid array-access & index
					printf("valid array\n");
					ret=json_array_get(ret,indices[i]);
				}
			}else{
				printf("invalid child\n");
				ret=NULL;
				break;
			}
			
		}
		
		free(new_created);
	}else{
		printf("invalid path\n");
		ret=NULL;
	}
	
	//free up memory
// 	free(indices);
	for(i=0;i<layers;i++){
		free(children[i]);
	}
	free(children);
	printf("Result: `%s`\n",json_string_value(ret));
	return ret;
}


int main(){
//  	char p[] = "$.c0:4a:00:ed:f1:bc.network.addresses[1]";
	char p[] = "$.c0:4a:00:ed:f1:bc.network.mac";
// 	printf("valid: %i\n", json_is_valid_path(p));
	json_t *tst_json = json_load_file("json.json", 0, NULL);
	printf("Result: `%s`\n",json_string_value(json_path_get(tst_json,p)));
	return 0;
}