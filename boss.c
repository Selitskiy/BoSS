#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <sys/time.h>
//#include <math.h>

#define BOF_STR_MAX 200
#define BOF_HASH_MAX 200
#define BOF_SHASH_LEN 189
#define BOF_HREG_LEN 27

int cmp_boss(char *hash1, char *hash2, double *pcosine_dist, double *peuclid_dist){
  int i;
  double product=0., norm1=0., norm2=0., dnorm=0., cosine_dist2, euclid_dist2;
  double l_cosine_dist_max = *pcosine_dist;
  double l_euclid_dist_max = *peuclid_dist;

  for(i=0; i<strlen(hash1) && i<strlen(hash2); ++i)
    {
      product += (hash1[i]-'0') * (hash2[i]-'0');
      norm1 += (hash1[i]-'0') * (hash1[i]-'0');
      norm2 += (hash2[i]-'0') * (hash2[i]-'0');
      dnorm += ((hash1[i]-'0') - (hash2[i]-'0')) * ((hash1[i]-'0') - (hash2[i]-'0'));
    }

  cosine_dist2 = product * product / norm1 / norm2;
  euclid_dist2 = dnorm;

  if(pcosine_dist)
    *pcosine_dist = cosine_dist2; 
  if(peuclid_dist)
    *peuclid_dist = euclid_dist2; 

  printf("BoSS distance '%s'->'%s', prod %f norm1 %f norm2 %f cosine^2 %f euclid^2 %f\n", 
	 hash1, hash2, product, norm1, norm2, cosine_dist2, euclid_dist2);

  if( cosine_dist2 > l_cosine_dist_max*l_cosine_dist_max && euclid_dist2 < l_euclid_dist_max*l_euclid_dist_max )
    return 0;
  else
    return -1;

}

//Expected bof_str len BOF_STR_MAX 200, and hash BOF_HASH_MAX 32
void build_boss(char *str, char *bof_str, char *hash)
{
  int i;
  char curr;
  //Syllable hash
  int state; //0, 1, 2 - out of the word, on consonant, on vowel/end of the syllable
  int hreg, lreg; // higher (vowel) register (0-5), lower (consonant) register (0-26); 0 - null-consonant or non-vowel single letter syllable

  if(!str || !bof_str || ! hash)
    return;

  strlcpy(bof_str, str, BOF_STR_MAX+1);
  memset(hash, 0, BOF_HASH_MAX+1);

  // Syllable hash
  memset(hash, '0', BOF_SHASH_LEN);
  hreg = lreg = 0;
  for(state = 0, i=0; i<strlen(bof_str); ++i)
    {
      curr = tolower(bof_str[i]);
      bof_str[i] = curr;


      if(!state)
	{
	  //enterig word/syllable from space or from finished syllable

	  if(curr >= 'a' && curr <= 'z')
	    {
	      if(curr == 'a' || curr == 'i' || curr == 'u' || curr == 'e' || curr == 'o' || curr == 'y')
		{
		  //null-consonant syllable
		  if(curr == 'a')
		    hreg = 1;
		  else if(curr == 'i')
		    hreg = 2;
		  else if(curr == 'u')
		    hreg = 3;
		  else if(curr == 'e')
		    hreg = 4;
		  else if(curr == 'o')
		    hreg = 5;
		  else if(curr == 'y')
		    hreg = 6;

		  ++hash[hreg*BOF_HREG_LEN];

		  hreg = lreg = 0;
		  state = 0;
		}
	          else
		    {
		      //beginning consonant - we'll wait for the closing vowel, another consonant or space
		      lreg = curr - 'a';
		      
		      hreg = 0;
		      state = 1;
		    }
	    }
	  //else - yet another space - do nothing

	}
          else
	    {
	      //got consonant on previous step, now look for end of syllable (vowel or next syllable consonant)
	      if(curr >= 'a' && curr <= 'z')
		{
		  if(curr == 'a' || curr == 'i' || curr == 'u'  || curr == 'e'  || curr == 'o' || curr == 'y')
		    {
		      //closing full consonant-vowel syllable
		      if(curr == 'a')
			hreg = 1;
		      else if(curr == 'i')
			hreg = 2;
		      else if(curr == 'u')
			hreg = 3;
		      else if(curr == 'e')
			hreg = 4;
		      else if(curr == 'o')
			hreg = 5;
		      else if(curr == 'y')
			hreg = 6;

		      ++hash[hreg*BOF_HREG_LEN+lreg];

		      hreg = lreg = 0;
		      state = 0;      
		    }
		      else
			{
			  //another consonant, i.e syllable
			  //finish null-vowel syllable

			  ++hash[lreg];

			  lreg = curr - 'a';
			  hreg = 0;
			  state = 1;
			}
		}
	      else{
		// space - have to finish null-vowel syllable
		  
		++hash[lreg];

		hreg = lreg = 0;
		state = 0;
	      }

	    } //state

    }
  
  if(state)
    {
      ++hash[lreg];
    }

}



int main(int argc, char **argv)
{
  char bof1[BOF_STR_MAX+1], bof2[BOF_STR_MAX+1];
  char hash1[BOF_HASH_MAX+1], hash2[BOF_HASH_MAX+1];
  double cdist2=0., edist2=0.;
  struct timeval tv1, tv2;

  if(argc < 3){
    printf("Usage: boss \"str1\" \"str2\", |str| <= 200\n");
    exit(-1);
  }

  strcpy(bof1, argv[1]);
  strcpy(bof2, argv[2]);

  //strcpy(bof1, "donald: sprucing up for spring");
  //strcpy(bof2, "vulindlela: sprucing up for spring");

  gettimeofday(&tv1, NULL);
  build_boss(bof1, bof1, hash1);
  gettimeofday(&tv2, NULL);

  build_boss(bof2, bof2, hash2);

  printf("build_boss: str1 '%s', trunk str1 '%s', hash1 '%s', time %ld us\n", 
	 bof1, bof1, hash1, (tv2.tv_sec-tv1.tv_sec)*1000000+tv2.tv_usec-tv1.tv_usec);

  gettimeofday(&tv1, NULL);
  cmp_boss(hash1, hash2, &cdist2, &edist2);
  gettimeofday(&tv2, NULL);

  printf("cmp_boss: time %ld us\n", 
	 (tv2.tv_sec-tv1.tv_sec)*1000000+tv2.tv_usec-tv1.tv_usec);
}
