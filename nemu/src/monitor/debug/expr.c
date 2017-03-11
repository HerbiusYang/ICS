#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
	NOTYPE = 256, EQ , AND , OR , NO , HEX , NUM , REG , LS , RS , LE , ME , NE , LEA , EX ,
	/* TODO: Add more token types */

};

static struct rule {
	char *regex;
	int token_type;
} rules[] = {

	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */

	{"[0-9]+", NUM},					//10 scale number of regular expression 
	{"0[Xx][0-9a-fA-F]+", HEX},			//16 scale number of regular expression
										//only make the 10scale and 16scale's calculated succeed
	{"\\$[[:alpha:]]+", REG},           //$ of expression 
	{"[[:alpha:][:digit:]]+", EX},     //judge the double symbol


	{"\\+", '+'},					//plus
	{"-", '-'},                     //subtract
	{"\\*", '*'},					//mulitply
	{"\\/", '/'},					//divide
	{"\\(", '('},					//left bracket
	{"\\)", ')'},                   //right bracket


	{"&&", AND},					//And 
	{"\\|\\|", OR},					//OR
	{"!", NO},						//NOT
	{"%", '%'},						//complementation
	{"<<", LS},						//Left shift
	{">>", RS},						//Right shift
	{"&", '&'},						//And binary
	{"\\|", '|'},					//OR calculate

	{">", '>'},						//symbol >
	{"<", '<'},						//symbol <
	{"<=", LE},						//less than or euqual to 
	{">=", ME},						//More than or euqual to
	{"!=", NE},						//Not Equal
	{"\\^", '^'},					//XOR
	//{"*", LEA},						//index variety



//Demo:
	{" +",	NOTYPE},				// a seris of spaces
	{"==", EQ},						// equal
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )      //Maybe judeg the rules array's max storage HER

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
	int i;
	char error_msg[128];
	int ret;

	for(i = 0; i < NR_REGEX; i ++) {
		ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
		if(ret != 0) {
			regerror(ret, &re[i], error_msg, 128);
			Assert(ret == 0, "regex compilation failed: %s\n%s", error_msg, rules[i].regex);
		}
	}
}

typedef struct token {
	int type;
	char str[32];
} Token;

Token tokens[32];   //This is the token array the expession must read-in HER max storage

int nr_token;   //define the label of tokens array position HER

//This part should recognize seriously...



static bool make_token(char *e) {
	int position = 0;
	int i;
	regmatch_t pmatch;
	
	nr_token = 0;

	while(e[position] != '\0') {
		/* Try all rules one by one. */
		for(i = 0; i < NR_REGEX; i ++) {
			if(regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
				char *substr_start = e + position;
				int substr_len = pmatch.rm_eo;

				Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i, rules[i].regex, position, substr_len, substr_len, substr_start);
				position += substr_len;

//ADD Part
				/* TODO: Now a new token is recognized with rules[i]. Add codes
				 * to record the token in the array ``tokens''. For certain 
				 * types of tokens, some extra actions should be performed.
				 */
				switch(rules[i].token_type) {
					case NUM: case HEX: case REG: case EX:
						tokens[nr_token].type=rules[i].token_type;  //input the information into the tokens array HER
						strncpy(tokens[nr_token].str,substr_start,substr_len);
						strncpy(tokens[nr_token].str+substr_len,"\0",1);
						nr_token++;
						break;
					case NOTYPE:									//if the expression's character is empty so break the function HER
						break;
					default:
						tokens[nr_token].type=rules[i].token_type;
						nr_token++;
				}
				break;
			}
		}
		if(i == NR_REGEX) {
			printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
			return false;
		}
	}

	return true; 
}

bool check_parentheses(int p, int q , bool *success)
{
	int i;
	bool flag=((tokens[p].type =='(') && (tokens[q].type==')')); //judge the front expression and base expression have the buckets or not
	int  sum=0;
	for(i=p;i<=q;i++){
		if(tokens[i].type == '(')
			++sum;
		if(tokens[i].type == ')')
			--sum;
		if(sum<=-1)
			success = false;
	}
	if(sum != 0)
		success = false;
	return success && flag;

}

 
int level(int type)   //Judge the symbol level for calculation....
{
	int index = 100; // Empty initial number
	switch(type){
		case NO : case LEA:
			index = 20;break;
		case '*': case '/': case '%':
			index = 19;break;
		case '+': case '-':
			index = 18;break;
		case  LS: case  RS:
			index = 17;break;
		case '<': case '>': case ME: case LE:
			index = 16;break;
		case  EQ: case  NE:
			index = 15;break;
		case '&':
			index = 14;break;
		case '^':
			index = 13;break;
		case '|':
			index = 12;break;
		case  AND:
			index = 11;break;
		case  OR:
			index = 10;break;
	}
	return index;
}

int dominant(int p,int q ) {
	int i,sum=0,location=-1;
	for (i=p;i<=q;i++) {
		switch (tokens[i].type) {
			case '(' :
				++sum;
				break;

			case ')' :
				--sum;
				break;

			case NUM :
				break;

			default :
				if (sum==0) {
					if (location<0) 
						location=i;
					if (level(tokens[i].type) < level(tokens[location].type))
						switch (tokens[i].type) {
							 case NO: //case LEA:
								break;
							default :
								location=i;  //if not the symbol the location of expression must be return to initial location  HER 
						}
				}
		}
	}
	return location;
}


uint32_t eval(int p,int q,bool *success) {


	    if(p > q) {
		/* Bad expression */
			*success=false;    //IF the index number of expression get wrong it must be weong ... return 0 HER
			return 0;
		}
		else if(p == q) { 
		/* Single token.
		* For now this token should be a number. 
		* Return the value of the number.
		*/ 
			uint32_t value=0;
			int i;
			bool flag=false;

			switch (tokens[p].type) {
				case NUM:
					sscanf(tokens[p].str,"%d",&value);//input the 10-scale number
					break;

				case HEX:
					sscanf(tokens[p].str,"%x",&value);//input the HEX number
					break;

				case SYM:{
					swaddr_t addr;
					addr = find_sym(tokens[p].str);
					if(addr != 0)
						return addr;
					else 
						*success = false;
					break;
				}

				case REG:
					for (i=R_EAX;i<=R_EDI;i++)
						if (strcasecmp(regsl[i],tokens[p].str+1)==0) {
							value=reg_l(i);
							flag=true;
							break;
						}
					for (i=R_AX;i<=R_DI;i++)
						if (strcasecmp(regsw[i],tokens[p].str+1)==0) {
							value=reg_w(i);
							flag=true;
							break;
						}
					for (i=R_AL;i<=R_BH;i++)
						if (strcasecmp(regsb[i],tokens[p].str+1)==0) {
							value=reg_b(i);
							flag=true;
							break;
						}
					if (strcasecmp("eip",tokens[p].str+1)==0) {
						value=cpu.eip;
						flag=true;
					}
					if (flag==false)
						*success=false;
					break;

				default :
					*success=false;
			}
			return value;
		}
		else if(check_parentheses(p,q,success)==true) {
		/* The expression is surrounded by a matched pair of parentheses. 
		 * If that is the case, just throw away the parentheses.	 
		 */
			return eval(p+1,q-1,success); 
		}
		else if (*success==true) {

			int op=dominant(p,q);

			uint32_t val1;

			if (op==p) 
				val1=0;
			else 
				val1=eval(p,op-1,success);

			uint32_t val2=eval(op+1,q,success);

			switch (tokens[op].type) {         //calculate part HER

				case '+': return val1+val2; 
				case '-': return val1-val2;
				case '*': return val1*val2;
				case '/': 
					if (val2!=0) 
						return val1/val2;
					else {
						*success=false;
						return 0; 
					}
				case '%': return val1%val2;
				case '<': return val1<val2;
				case '>': return val1>val2;
				case LE : return val1<=val2;
				//case LEA: return swaddr_read(val2,4);
				case ME : return val1>=val2;
				case EQ : return val1==val2;
				case NE : return val1!=val2;
				case AND: return val1&&val2;
				case OR : return val1||val2;
				case NO:  return !val2;
				case LS:  return val1<<val2;
				case RS:  return val1>>val2;
				case '&': return val1&val2;
				case '|': return val1|val2;
				case '^': return val1^val2;
	
				default: 
					*success=false; 
					assert(0);
					return 0;
			}
		}
		else 
			return 0;
}


void print_token() {
	int tmp=0;
		for (;tmp<nr_token;tmp++) {
			if (tokens[tmp].type==NUM||tokens[tmp].type==HEX||tokens[tmp].type==REG||tokens[tmp].type==SYM)
				printf("%s",tokens[tmp].str);
			else 
				switch (tokens[tmp].type) {
					case LS: printf("<<"); break;
					case RS: printf(">>"); break;
					case LE: printf("<="); break;
					case ME: printf(">="); break;
					case EQ: printf("=="); break;
					case NE: printf("!="); break;
					case AND: printf("&&"); break;
					case OR : printf("||"); break;
					case NO: printf("!"); break;
					default:
						printf("%c",tokens[tmp].type);
				}
		}
	printf(" = ");
}



uint32_t expr(char *e, bool *success) {
	int i;
	if(!make_token(e)) {
		*success = false;
		return 0;
	}

	/* TODO: Insert codes to evaluate the expression. */
	/*for (i=0;i<nr_token;i++) {
		if (tokens[i].type=='*') {
			if (tokens[i-1].type != NUM && tokens[i-1].type != HEX && tokens[i-1].type!=REG)
				tokens[i].type=LEA;
		}
	} //judge the index expression or symbol of multiplication

	*/

	uint32_t result=eval(0,nr_token-1,success);

	if (*success==false) {
		printf("Calaulate failed!\n");
		return -1;
	}
	else { 
		return result; 
	}

	panic("please implement me");

	return 0;
}



