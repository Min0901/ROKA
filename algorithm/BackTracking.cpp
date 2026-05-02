#include <iostream>

using namespace std;

int n,cnt=0;
int arr[16][16] = {};

void Solve(int d)
{
    if(d==n)
        ++cnt;
    for(int i=0;i<n;++i){
        for(int j=0;j<n;++j){
            if(arr[i][j] == -1)
                continue;

            int arr2[16][16] = {};
            for(int y=0; y<n; ++y){
                for(int x=0; x<n; ++x) 
                    arr2[y][x] = arr[y][x];
            }

            for(int k=0;k<n;++k){
                arr[i][k] = -1;
                arr[k][j] = -1;
            }
            //수평, 수직
            
            int dx,dy;
            dx=-1; dy=-1;
            while(i+dy>0 && j+dx>0)
                arr[i+(dy--)][j+(dx--)] = -1;
            dx=1; dy=-1;
            while(i+dy>0 && j+dx<n)
                arr[i+(dy--)][j+(dx++)] = -1;
            dx=-1; dy=1;
            while(i+dy<n && j+dx>0)
                arr[i+(dy++)][j+(dx--)] = -1;
            dx=1; dy=1;
            while(i+dy<n && j+dx<n)
                arr[i+(dy++)][j+(dx++)] = -1;
            //대각선(좌상,우상,좌하,우하)
            
            Solve(d+1);

            for(int y=0; y<n; ++y){
                for (int x=0; x<n; ++x) 
                    arr[y][x] = arr2[y][x];
            }
            //원상태 복구
        }
    }
}

int main() {
    cin >> n;

    Solve(0);

    cout << cnt;
    
    return 0;
}