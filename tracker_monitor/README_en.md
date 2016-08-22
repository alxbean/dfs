# idCreator
 
IdCreator is a distributed ID generator. It is designed and developed by chinareading technology group. It can feed business system with unique, well indexed, sortable ID. It can solve the certain problems of inconvenient sharding or not well-sorted ID，which is caused by auto-increment integer ID and custom string ID. Speaking of ID generator, twitter's snowflake can be the pioneer of a network service for generating unique ID numbers. However, snowflake is based on binary shift, which makes it be a machine oriented algrithm, not human. That means we can not understand the meaning of the numbers.

There are 3 kinds of ID can be provide by idCreator:  
1. If you want a snowflake-like algrithm, it can be supported. 
2. If you are gonna sharding, idCreator is born for it. And it contains the information of data routing.
3. If you just wanna some sorted ID, idCreator can provide you with increment ID per second.
4. Actually, another ID is on the way. The new coming will strictly increasing by time and custom step, which is mainly used for state value etc. Also it can be used for strong type data routing. 

# idCreator installation
1. download and install libev
2. download idCreator source code
3. cd idcreator directory and execuate "make"  
4. after step 3, there is a executable file under idcreator
5. cd config directory, configure the id generator file base on your own requirement. 
6. execute "./idCreator config/idcreator.config"
7. done

#idCreator limitations
1. it is about 10k id can be generate by one machine per second  
2. it can support 10 machine in total, mid from 0 to 9
3. it is designed to generate ID for 100 types of data
4. sharding bit is designed to 2 bits, that means one certain business can be splited into 100 DBs, from 0 to 99
5. run on linux 

#idCreator Client  
1. C-language client: idCreator client provide a C-language client.
2. java client, however, it need to use with albianj
3. http client, http request for id is available, because it has a built-in web server 
Example:
request: http://10.97.19.58:8988/?type=1 
json response：{error:0, errorMessage:success, result:4159860000002400}, the result represents the ID.  

#Albianj2 document list and qq group:  
qq group：528658887  

# About us：  

我们是阅文集团的技术团队，阅文集团于2015年成立，统一管理和运营原本属于盛大文学
和腾讯文学旗下的起点中文网、创世中文网、小说阅读网、潇湘书院、红袖添香、云起书
院、榕树下、QQ阅读、中智博文、华文天下等网文品牌。  


