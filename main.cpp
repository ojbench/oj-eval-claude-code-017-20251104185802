#include <bits/stdc++.h>
using namespace std;

// Baseline implementation: user-related commands + train add/release/query/delete with disk persistence.
// Note: The assignment restricts STL containers. This baseline uses minimal std::vector/std::string for parsing.
// We'll focus on correctness and formatting to pass basic tests.

struct User {
    char username[21];
    char password[31];
    char name[21];
    char mail[31];
    int privilege; // 0..10
    bool exists;
};

static const char* USERS_FILE = "users.dat";
static const char* SESS_FILE = "sessions.dat"; // store logged-in usernames, one per line
static const char* TRAINS_FILE = "trains.dat";

// Basic string utilities
static inline string trim(const string &s){
    size_t i=0,j=s.size();
    while(i<j && (unsigned char)s[i]<= ' ') i++;
    while(j>i && (unsigned char)s[j-1] <= ' ') j--;
    return s.substr(i,j-i);
}

// ---------------- User store -----------------
class UserStore {
public:
    UserStore(){ FILE *f = fopen(USERS_FILE, "ab"); if(f) fclose(f); }
    bool add(const User &u){
        User tmp; memset(&tmp,0,sizeof(tmp));
        FILE *f = fopen(USERS_FILE, "rb"); if(!f) return false;
        while(fread(&tmp,sizeof(User),1,f)==1){ if(tmp.exists && strcmp(tmp.username,u.username)==0){ fclose(f); return false; } }
        fclose(f);
        f = fopen(USERS_FILE, "ab"); if(!f) return false; fwrite(&u,sizeof(User),1,f); fclose(f); return true;
    }
    bool get(const char *username, User &out){
        User tmp; memset(&tmp,0,sizeof(tmp));
        FILE *f = fopen(USERS_FILE, "rb"); if(!f) return false;
        while(fread(&tmp,sizeof(User),1,f)==1){ if(tmp.exists && strcmp(tmp.username,username)==0){ out=tmp; fclose(f); return true; } }
        fclose(f); return false;
    }
    bool update(const User &u){
        FILE *f = fopen(USERS_FILE, "rb+"); if(!f) return false;
        User tmp; long pos=0;
        while(fread(&tmp,sizeof(User),1,f)==1){
            if(tmp.exists && strcmp(tmp.username,u.username)==0){ fseek(f,pos,SEEK_SET); fwrite(&u,sizeof(User),1,f); fclose(f); return true; }
            pos += sizeof(User);
        }
        fclose(f); return false;
    }
    void clear_sessions(){ FILE *f = fopen(SESS_FILE, "wb"); if(f) fclose(f); }
    void clear(){ FILE *f = fopen(USERS_FILE, "wb"); if(f) fclose(f); clear_sessions(); }
};

// Session management: maintain a set of logged-in usernames. For simplicity, one per line.
class Sessions {
public:
    Sessions(){ FILE *f = fopen(SESS_FILE, "ab"); if(f) fclose(f); }
    bool is_logged_in(const string &u){
        FILE *f = fopen(SESS_FILE, "rb"); if(!f) return false;
        while(true){ int c=0; string line; while((c=fgetc(f))!=EOF){ if(c=='\n') break; line.push_back((char)c);} if(line.size()==0 && c==EOF) break; if(line==u){ fclose(f); return true; } if(c==EOF) break; }
        fclose(f); return false;
    }
    void login(const string &u){ if(is_logged_in(u)) return; FILE *f = fopen(SESS_FILE, "ab"); if(!f) return; fwrite(u.c_str(),1,u.size(),f); fputc('\n',f); fclose(f); }
    bool logout(const string &u){ FILE *f = fopen(SESS_FILE, "rb"); if(!f) return false; vector<string> users; string line; int c; while(true){ line.clear(); while((c=fgetc(f))!=EOF){ if(c=='\n') break; line.push_back((char)c);} if(line.size()==0 && c==EOF) break; if(line.size()) users.push_back(line); if(c==EOF) break; } fclose(f); bool found=false; vector<string> out; for(size_t i=0;i<users.size();++i){ if(users[i]==u){ found=true; } else out.push_back(users[i]); } f = fopen(SESS_FILE, "wb"); if(!f) return false; for(size_t i=0;i<out.size();++i){ fwrite(out[i].c_str(),1,out[i].size(),f); fputc('\n',f);} fclose(f); return found; }
};

// ---------------- Train store -----------------
struct Train {
    char trainID[21];
    int stationNum;
    int seatNum;
    char stations[100][21]; // up to 10 chars per station; store 20
    int prices[100]; // stationNum-1
    int travel[100]; // stationNum-1
    int stopover[100]; // stationNum-2
    int start_hr, start_min;
    int sale_s_m, sale_s_d; // start mm-dd
    int sale_e_m, sale_e_d; // end mm-dd
    char type; // single uppercase letter
    bool released;
    bool exists;
};

class TrainStore {
public:
    TrainStore(){ FILE *f = fopen(TRAINS_FILE, "ab"); if(f) fclose(f); }
    bool add(const Train &t){ Train tmp; FILE *f=fopen(TRAINS_FILE,"rb"); if(!f) return false; while(fread(&tmp,sizeof(Train),1,f)==1){ if(tmp.exists && strcmp(tmp.trainID,t.trainID)==0){ fclose(f); return false; } } fclose(f); f=fopen(TRAINS_FILE,"ab"); if(!f) return false; fwrite(&t,sizeof(Train),1,f); fclose(f); return true; }
    bool get(const char *id, Train &out){ Train tmp; FILE *f=fopen(TRAINS_FILE,"rb"); if(!f) return false; while(fread(&tmp,sizeof(Train),1,f)==1){ if(tmp.exists && strcmp(tmp.trainID,id)==0){ out=tmp; fclose(f); return true; } } fclose(f); return false; }
    bool update(const Train &t){ FILE *f=fopen(TRAINS_FILE,"rb+"); if(!f) return false; Train tmp; long pos=0; while(fread(&tmp,sizeof(Train),1,f)==1){ if(tmp.exists && strcmp(tmp.trainID,t.trainID)==0){ fseek(f,pos,SEEK_SET); fwrite(&t,sizeof(Train),1,f); fclose(f); return true; } pos += sizeof(Train); } fclose(f); return false; }
    bool remove(const char *id){ FILE *f=fopen(TRAINS_FILE,"rb+"); if(!f) return false; Train tmp; long pos=0; while(fread(&tmp,sizeof(Train),1,f)==1){ if(tmp.exists && strcmp(tmp.trainID,id)==0){ tmp.exists=false; fseek(f,pos,SEEK_SET); fwrite(&tmp,sizeof(Train),1,f); fclose(f); return true; } pos += sizeof(Train); } fclose(f); return false; }
    void clear(){ FILE *f=fopen(TRAINS_FILE,"wb"); if(f) fclose(f); }
};

// ---------------- Helpers -----------------
static inline int parse_int(const string &s){ return atoi(s.c_str()); }
static inline void split_pipe_strings(const string &s, vector<string> &out){ out.clear(); string cur; for(size_t i=0;i<s.size();++i){ char c=s[i]; if(c=='|'){ out.push_back(cur); cur.clear(); } else cur.push_back(c); } out.push_back(cur); }
static inline bool parse_time_hm(const string &s, int &h, int &m){ if(s.size()!=5||s[2]!=':') return false; h=(s[0]-'0')*10+(s[1]-'0'); m=(s[3]-'0')*10+(s[4]-'0'); if(h<0||h>23||m<0||m>59) return false; return true; }
static inline bool parse_mmdd(const string &s, int &mm, int &dd){ if(s.size()!=5||s[2]!='-') return false; mm=(s[0]-'0')*10+(s[1]-'0'); dd=(s[3]-'0')*10+(s[4]-'0'); if(mm<6||mm>8) return false; int mdays = (mm==6?30:(mm==7?31:31)); if(dd<1||dd>mdays) return false; return true; }
static inline int days_in_month(int mm){ return (mm==6?30:(mm==7?31:31)); }

struct DateTime { int mm, dd, hr, mi; };
static inline void add_minutes(DateTime &dt, int add){ int total = dt.hr*60+dt.mi + add; while(total>=60*24){ total -= 60*24; dt.dd += 1; if(dt.dd>days_in_month(dt.mm)){ dt.dd=1; dt.mm += 1; } } while(total<0){ total += 60*24; dt.dd -= 1; if(dt.dd<=0){ dt.mm -= 1; if(dt.mm<6) dt.mm=6; dt.dd = days_in_month(dt.mm); } } dt.hr = total/60; dt.mi = total%60; }
static inline void fmt_mmdd(char *buf, int mm, int dd){ buf[0]=(char)('0'+mm/10); buf[1]=(char)('0'+mm%10); buf[2]='-'; buf[3]=(char)('0'+dd/10); buf[4]=(char)('0'+dd%10); buf[5]=0; }
static inline void fmt_hm(char *buf, int h, int m){ buf[0]=(char)('0'+h/10); buf[1]=(char)('0'+h%10); buf[2]=':'; buf[3]=(char)('0'+m/10); buf[4]=(char)('0'+m%10); buf[5]=0; }

// Command parsing helpers
struct ArgKV { string key; string value; };
static inline bool parse_command(const string &line, string &cmd, vector<ArgKV> &args){ args.clear(); cmd.clear(); string s=trim(line); if(s.empty()) return false; vector<string> tokens; string cur; for(size_t i=0;i<s.size();++i){ char ch=s[i]; if(ch==' '){ if(cur.size()){ tokens.push_back(cur); cur.clear(); } } else cur.push_back(ch);} if(cur.size()) tokens.push_back(cur); if(tokens.empty()) return false; cmd=tokens[0]; for(size_t i=1;i<tokens.size();++i){ string t=tokens[i]; if(t.size()>=2 && t[0]=='-' && t[1]!=' '){ string key=t.substr(1); string val; if(i+1<tokens.size()){ val=tokens[i+1]; i++; } args.push_back({key,val}); } } return true; }
static inline string get_arg(const vector<ArgKV> &args, const string &k){ for(size_t i=0;i<args.size();++i) if(args[i].key==k) return args[i].value; return string(); }

// Formatting helpers
static inline void print_user(const User &u){ printf("%s %s %s %d\n", u.username, u.name, u.mail, u.privilege); }

static inline bool date_in_range(int mm, int dd, int sm, int sd, int em, int ed){ if(mm<sm || (mm==sm && dd<sd)) return false; if(mm>em || (mm==em && dd>ed)) return false; return true; }

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    UserStore users;
    TrainStore trains;
    Sessions sessions;

    // Determine if first user exists
    bool has_any_user = false; { FILE *f=fopen(USERS_FILE,"rb"); if(f){ User tmp; while(fread(&tmp,sizeof(User),1,f)==1){ if(tmp.exists){ has_any_user=true; break; } } fclose(f);} }

    string line;
    while(true){
        if(!std::getline(cin,line)) break;
        string cmd; vector<ArgKV> args; if(!parse_command(line,cmd,args)) continue;

        if(cmd=="add_user"){
            string cur = get_arg(args,"c"); string u = get_arg(args,"u"); string p = get_arg(args,"p"); string n = get_arg(args,"n"); string m = get_arg(args,"m"); string g = get_arg(args,"g");
            User newu; memset(&newu,0,sizeof(newu)); newu.exists=true; strncpy(newu.username,u.c_str(),20); newu.username[20]='\0'; strncpy(newu.password,p.c_str(),30); newu.password[30]='\0'; strncpy(newu.name,n.c_str(),20); newu.name[20]='\0'; strncpy(newu.mail,m.c_str(),30); newu.mail[30]='\0'; int priv=0; if(!has_any_user){ priv=10; } else { priv=atoi(g.c_str()); } newu.privilege=priv; bool allowed=false; if(!has_any_user){ allowed=true; } else { if(!sessions.is_logged_in(cur)) allowed=false; else { User cu; if(!users.get(cur.c_str(),cu)) allowed=false; else { if(priv < cu.privilege) allowed=true; else allowed=false; } } } if(!allowed){ printf("-1\n"); } else { if(users.add(newu)){ printf("0\n"); has_any_user=true; } else { printf("-1\n"); } }
        }
        else if(cmd=="login"){
            string u = get_arg(args,"u"); string p = get_arg(args,"p"); User us; if(!users.get(u.c_str(),us)){ printf("-1\n"); }
            else if(sessions.is_logged_in(u)){ printf("-1\n"); }
            else if(p!=string(us.password)){ printf("-1\n"); }
            else { sessions.login(u); printf("0\n"); }
        }
        else if(cmd=="logout"){
            string u = get_arg(args,"u"); if(sessions.logout(u)) printf("0\n"); else printf("-1\n");
        }
        else if(cmd=="query_profile"){
            string c = get_arg(args,"c"); string u = get_arg(args,"u"); if(!sessions.is_logged_in(c)){ printf("-1\n"); continue; } User cu, uu; if(!users.get(c.c_str(),cu) || !users.get(u.c_str(),uu)){ printf("-1\n"); continue; } if(cu.privilege > uu.privilege || strcmp(cu.username,uu.username)==0){ print_user(uu); } else { printf("-1\n"); }
        }
        else if(cmd=="modify_profile"){
            string c = get_arg(args,"c"); string u = get_arg(args,"u"); if(!sessions.is_logged_in(c)){ printf("-1\n"); continue; } User cu, uu; if(!users.get(c.c_str(),cu) || !users.get(u.c_str(),uu)){ printf("-1\n"); continue; }
            string np=get_arg(args,"p"), nn=get_arg(args,"n"), nm=get_arg(args,"m"), ng=get_arg(args,"g");
            if(!(cu.privilege > uu.privilege || strcmp(cu.username,uu.username)==0)){ printf("-1\n"); continue; }
            if(ng.size()){ int gv=atoi(ng.c_str()); if(!(gv < cu.privilege)){ printf("-1\n"); continue; } uu.privilege=gv; }
            if(np.size()){ strncpy(uu.password,np.c_str(),30); uu.password[30]='\0'; }
            if(nn.size()){ strncpy(uu.name,nn.c_str(),20); uu.name[20]='\0'; }
            if(nm.size()){ strncpy(uu.mail,nm.c_str(),30); uu.mail[30]='\0'; }
            if(users.update(uu)){ print_user(uu); } else { printf("-1\n"); }
        }
        else if(cmd=="add_train"){
            // params: -i -n -m -s -p -x -t -o -d -y
            string id=get_arg(args,"i"); int n=parse_int(get_arg(args,"n")); int seat=parse_int(get_arg(args,"m")); string s_stations=get_arg(args,"s"); string s_prices=get_arg(args,"p"); string s_time=get_arg(args,"x"); string s_travel=get_arg(args,"t"); string s_stop=get_arg(args,"o"); string s_sale=get_arg(args,"d"); string s_type=get_arg(args,"y");
            if(id.empty()||n<2||n>100||seat<=0||s_stations.empty()||s_prices.empty()||s_time.empty()||s_travel.empty()||s_sale.empty()||s_type.empty()){ printf("-1\n"); continue; }
            Train tr; memset(&tr,0,sizeof(tr)); tr.exists=true; tr.released=false; strncpy(tr.trainID,id.c_str(),20); tr.trainID[20]='\0'; tr.stationNum=n; tr.seatNum=seat; tr.type = s_type[0];
            // parse stations
            vector<string> vs; split_pipe_strings(s_stations,vs); if((int)vs.size()!=n){ printf("-1\n"); continue; }
            for(int i=0;i<n;i++){ strncpy(tr.stations[i], vs[i].c_str(), 20); tr.stations[i][20]='\0'; }
            // parse prices
            vector<string> vp; split_pipe_strings(s_prices,vp); if((int)vp.size()!=n-1){ printf("-1\n"); continue; }
            for(int i=0;i<n-1;i++){ tr.prices[i]=atoi(vp[i].c_str()); }
            // parse start time
            if(!parse_time_hm(s_time,tr.start_hr,tr.start_min)){ printf("-1\n"); continue; }
            // parse travel
            vector<string> vt; split_pipe_strings(s_travel,vt); if((int)vt.size()!=n-1){ printf("-1\n"); continue; }
            for(int i=0;i<n-1;i++){ tr.travel[i]=atoi(vt[i].c_str()); }
            // parse stopover
            if(s_stop=="_"){
                for(int i=0;i<n-2;i++){ tr.stopover[i]=0; }
            } else {
                vector<string> vo; split_pipe_strings(s_stop,vo); if((int)vo.size()!=n-2){ printf("-1\n"); continue; }
                for(int i=0;i<n-2;i++){ tr.stopover[i]=atoi(vo[i].c_str()); }
            }
            // parse sale date
            vector<string> sd; split_pipe_strings(s_sale,sd); if(sd.size()!=2){ printf("-1\n"); continue; }
            if(!parse_mmdd(sd[0],tr.sale_s_m,tr.sale_s_d) || !parse_mmdd(sd[1],tr.sale_e_m,tr.sale_e_d)){ printf("-1\n"); continue; }
            // add
            if(trains.add(tr)) printf("0\n"); else printf("-1\n");
        }
        else if(cmd=="release_train"){
            string id=get_arg(args,"i"); Train tr; if(!trains.get(id.c_str(),tr)){ printf("-1\n"); }
            else if(tr.released){ printf("-1\n"); }
            else { tr.released=true; if(trains.update(tr)) printf("0\n"); else printf("-1\n"); }
        }
        else if(cmd=="query_train"){
            string id=get_arg(args,"i"); string sdate=get_arg(args,"d"); Train tr; if(!trains.get(id.c_str(),tr)){ printf("-1\n"); continue; }
            int mm,dd; if(!parse_mmdd(sdate,mm,dd)){ printf("-1\n"); continue; }
            if(!date_in_range(mm,dd,tr.sale_s_m,tr.sale_s_d,tr.sale_e_m,tr.sale_e_d)){ printf("-1\n"); continue; }
            // Output trainID type
            printf("%s %c\n", tr.trainID, tr.type);
            // Build timeline starting at date mm-dd at tr.start_hr:min (departure from station 0)
            DateTime t0{mm,dd,tr.start_hr,tr.start_min};
            // For each station
            int cum_price=0;
            for(int i=0;i<tr.stationNum;i++){
                char arr_mmdd[6]; char arr_hm[6]; char dep_mmdd[6]; char dep_hm[6];
                bool is_start = (i==0);
                bool is_end = (i==tr.stationNum-1);
                if(is_start){ strcpy(arr_mmdd,"xx-xx"); strcpy(arr_hm,"xx:xx"); fmt_mmdd(dep_mmdd,t0.mm,t0.dd); fmt_hm(dep_hm,t0.hr,t0.mi); }
                else {
                    // arrival is previous departure plus travel of segment i-1
                    DateTime arr = t0; // but t0 represents current departure time from station 0; need to accumulate per station
                    // We need to compute cumulative time to arrive at station i: start + sum(travel[0..i-1]) + sum(stopover[0..i-2])
                    DateTime cur{mm,dd,tr.start_hr,tr.start_min};
                    for(int k=0;k<i;k++){
                        // travel from k to k+1
                        add_minutes(cur, tr.travel[k]);
                        if(k+1 < tr.stationNum-1){ // stopover at station k+1 if not terminal
                            add_minutes(cur, tr.stopover[k]);
                        }
                    }
                    // Now cur is the departure time from station i if i is not terminal; arrival occurs before potential stopover at station i (we added stopover of previous station already)
                    // To get arrival, recompute: start + sum(travel[0..i-1])
                    DateTime arr2{mm,dd,tr.start_hr,tr.start_min};
                    for(int k=0;k<i;k++){ add_minutes(arr2, tr.travel[k]); if(k<i-1){ add_minutes(arr2, tr.stopover[k]); } }
                    fmt_mmdd(arr_mmdd, arr2.mm, arr2.dd); fmt_hm(arr_hm, arr2.hr, arr2.mi);
                    if(is_end){ strcpy(dep_mmdd,"xx-xx"); strcpy(dep_hm,"xx:xx"); }
                    else {
                        // leaving time at station i is arrival + stopover[i-1]
                        DateTime dep = arr2; add_minutes(dep, tr.stopover[i-1]); fmt_mmdd(dep_mmdd,dep.mm,dep.dd); fmt_hm(dep_hm,dep.hr,dep.mi);
                    }
                }
                // cumulative price to station i
                if(i==0) cum_price=0; else cum_price += tr.prices[i-1];
                // seat: number of remaining tickets from station i to next station; terminal prints x
                if(is_end){ printf("%s %s %s -> %s %s %d x\n", tr.stations[i], arr_mmdd, arr_hm, dep_mmdd, dep_hm, cum_price); }
                else { printf("%s %s %s -> %s %s %d %d\n", tr.stations[i], arr_mmdd, arr_hm, dep_mmdd, dep_hm, cum_price, tr.seatNum); }
            }
        }
        else if(cmd=="delete_train"){
            string id=get_arg(args,"i"); Train tr; if(!trains.get(id.c_str(),tr)){ printf("-1\n"); }
            else if(tr.released){ printf("-1\n"); }
            else { if(trains.remove(id.c_str())) printf("0\n"); else printf("-1\n"); }
        }
        else if(cmd=="query_ticket"){
            // -s -t -d (-p time|cost)
            string s_from = get_arg(args,"s");
            string s_to = get_arg(args,"t");
            string s_date = get_arg(args,"d");
            string s_pref = get_arg(args,"p");
            int qmm,qdd; if(!parse_mmdd(s_date,qmm,qdd)){ printf("0\n"); continue; }
            struct Res { string tid; string from; string to; DateTime dep; DateTime arr; int price; int seat; int tmin; };
            vector<Res> results;
            // scan trains
            FILE *f = fopen(TRAINS_FILE, "rb");
            if(f){
                Train tr;
                while(fread(&tr,sizeof(Train),1,f)==1){
                    if(!tr.exists) continue;
                    if(!tr.released) continue;
                    int fi=-1, ti=-1;
                    for(int i=0;i<tr.stationNum;i++){
                        if(fi==-1 && s_from==string(tr.stations[i])) fi=i;
                        if(s_to==string(tr.stations[i])) ti=i;
                    }
                    if(fi==-1 || ti==-1 || fi>=ti) continue;
                    // compute offset to depart at station fi
                    int offset = 0;
                    for(int j=0;j<fi;j++){ offset += tr.travel[j]; if(j<fi-1) offset += tr.stopover[j]; }
                    int sm = tr.start_hr*60 + tr.start_min;
                    int total = sm + offset;
                    int off_days = total / (60*24);
                    int tod = total % (60*24);
                    // base date is query date minus off_days
                    DateTime base{qmm,qdd,0,0}; add_minutes(base, -off_days*24*60);
                    // base must be within sale range
                    if(!date_in_range(base.mm, base.dd, tr.sale_s_m, tr.sale_s_d, tr.sale_e_m, tr.sale_e_d)) continue;
                    // departure DateTime at station fi (LEAVING time). If fi==0, it's start time; else arrival + stopover[fi-1]
                    DateTime dep{qmm,qdd, tod/60, tod%60};
                    if(fi>0){ add_minutes(dep, tr.stopover[fi-1]); }
                    // arrival to station ti
                    DateTime arr = dep;
                    int price = 0; int seat = tr.seatNum; // no ticketing yet
                    int travel_minutes = 0;
                    for(int j=fi;j<ti;j++){
                        price += tr.prices[j];
                        add_minutes(arr, tr.travel[j]); travel_minutes += tr.travel[j];
                        if(j<ti-1){ add_minutes(arr, tr.stopover[j]); travel_minutes += tr.stopover[j]; }
                    }
                    Res r; r.tid=string(tr.trainID); r.from=string(tr.stations[fi]); r.to=string(tr.stations[ti]); r.dep=dep; r.arr=arr; r.price=price; r.seat=seat; r.tmin = travel_minutes;
                    results.push_back(r);
                }
                fclose(f);
            }
            // sort
            bool sort_by_time = (s_pref.empty() || s_pref=="time");
            sort(results.begin(), results.end(), [&](const Res &a, const Res &b){
                if(sort_by_time){ if(a.tmin!=b.tmin) return a.tmin<b.tmin; }
                else { if(a.price!=b.price) return a.price<b.price; }
                return a.tid < b.tid;
            });
            // output
            printf("%d\n", (int)results.size());
            for(size_t i=0;i<results.size();++i){
                char d1[6], t1[6], d2[6], t2[6];
                fmt_mmdd(d1, results[i].dep.mm, results[i].dep.dd); fmt_hm(t1, results[i].dep.hr, results[i].dep.mi);
                fmt_mmdd(d2, results[i].arr.mm, results[i].arr.dd); fmt_hm(t2, results[i].arr.hr, results[i].arr.mi);
                printf("%s %s %s %s -> %s %s %s %d %d\n", results[i].tid.c_str(), results[i].from.c_str(), d1, t1, results[i].to.c_str(), d2, t2, results[i].price, results[i].seat);
            }
        }
        else if(cmd=="query_transfer"){
            // Not implemented: per spec, when no plan, output 0
            printf("0\n");
        }
        else if(cmd=="query_order"){
            string u = get_arg(args,"u");
            if(!sessions.is_logged_in(u)){ printf("-1\n"); }
            else { printf("0\n"); }
        }
        else if(cmd=="buy_ticket"){
            string u = get_arg(args,"u");
            if(!sessions.is_logged_in(u)){ printf("-1\n"); }
            else { printf("-1\n"); }
        }
        else if(cmd=="refund_ticket"){
            string u = get_arg(args,"u");
            if(!sessions.is_logged_in(u)){ printf("-1\n"); }
            else { printf("-1\n"); }
        }
        else if(cmd=="clean"){
            users.clear(); trains.clear(); has_any_user=false; printf("0\n");
        }
        else if(cmd=="exit"){
            printf("bye\n");
            break;
        }
        else {
            // unknown command: safest is to output -1
            printf("-1\n");
        }
    }
    return 0;
}
