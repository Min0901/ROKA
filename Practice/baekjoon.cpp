#include <iostream>
#include <deque>

using namespace std;

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);
    
    deque<pair<int>,<int>> dq;
    int n;
    cin >> n;

    for(int i=0;i<n;++i){
        int temp;
        cin >> temp;
        dq.push_back({temp,i+1});
    }
    while(n--){
        if(dq.front()>0){
            cout << dq[0].second;
            dq.pop_front();
            for(int i=0;i<n-1;++i){
                dq.push_back(dq.front());
                dq.pop_front();
            }
        }else{
            cout << dq[0].second;
            dq.pop_front();
            for(int i=0;i<n;++i){
                dq.push_front(dq.back());
                dq.pop_back();
            }
        }
    }
    
    return 0;
}