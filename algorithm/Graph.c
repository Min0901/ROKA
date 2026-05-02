#include <stdio.h>
#include <stdlib.h>

enum VisitMode{Visited, NotVisited};

typedef int ElementType;

typedef struct Vertex{
    ElementType Data;
    int Visited;
    int Index;

    struct Vertex* Next;
    struct Edge* AdjacencyList;
}Vertex;
/*
Vertex(정점) 구조체의 필드
1.Data - 데이터를 담는 필드
2.Next - 다음 정점을 가리키는 포인터
3.AdjacencyList - 인접 정점의 목록에 대한 포인터(즉 간선 목록임)
4.Visited - 순회 알고리즘에서 사용
5.Index - 정점의 인덱스(첫 번째 정점은 0, 두 번째 정점은 1, ...같은 식으로) 최단 경로 탐색 알고리즘에서 사용
*/

typedef struct Edge{
    int Weight;
    struct Edge* Next;
    Vertex* From;
    Vertex* Target;
}Edge;
/*
Edge(간선) 구조체의 필드
1.From - 간선의 시작 정점
2.Target - 끝 정점
3.Next - 다음 간선을 가리키는 포인터(Vertex 구조체의 AdjacencyList가 Edge 구조체의 Next 포인터를 이용해 구성됨)
4.Weight - 간선의 가중치, 최소 신장 트리나 최단 경로 탐색 알고리즘에서 정점 사이의 거리나 비용 등을 표현하기 위해 사용
*/

typedef struct Graph{
    Vertex* Vertices;
    int VertexCount;
}Graph;
/*
Graph 구조체의 필드
1.Vertices - 정점 목록에 대한 포인터
2.VertexCount - 정점 수
*/

Graph* CreateGraph();
void DestroyGraph(Graph* G);
Vertex* CreateVertex(ElementType Data);
void DestroyVertex(Vertex* V);
Edge* CreateEdge(Vertex* From, Vertex* Target, int Weight);
void DestroyEdge(Edge* E);

Graph* CreateGraph(){
    Graph* graph = (Graph*)malloc(sizeof(Graph));
    graph->Vertices = NULL;
    graph->VertexCount = 0;

    return graph;
}

void DestroyGraph(Graph* G){
    while(G->Vertices != NULL){
        Vertex* Vertices = G->Vertices->Next;
        DestroyVertex(G->Vertices);
        G->Vertices = Vertices;
    }

    free(G);
}

Vertex* CreateVertex(ElementType Data){
    Vertex* V = (Vertex*)malloc(sizeof(Vertex));

    V->Data = Data;
    V->Next = NULL;
    V->AdjacencyList = NULL;
    V->Visited = NotVisited;
    V->Index = -1;

    return V;
}

void DestroyVertex(Vertex* V){
    while(V->AdjacencyList != NULL){
        Edge* Edge = V->AdjacencyList->Next;

        DestroyEdge(V->AdjacencyList);

        V->AdjacencyList = Edge;
    }

    free(V);
}

Edge* CreateEdge(Vertex* From, Vertex* Target, int Weight){
    Edge* E = (Edge*)malloc(sizeof(Edge));
    E->From     = From;
    E->Target   = Target;
    E->Next     = NULL;
    E->Weight   = Weight;

    return E;
}

void DestroyEdge(Edge* E){
    free(E);
}

void AddVertex(Graph* G, Vertex* V){
    Vertex* VertexList = G->Vertices;

    if(VertexList == NULL){
        G->Vertices = V;
    }else{
        while(VertexList->Next != NULL){
            VertexList = VertexList->Next;
        }

        VertexList->Next = V;
    }

    V->Index = G->VertexCount++;
}

void AddEdge(Vertex* V, Edge* E){
    if(V->AdjacencyList == NULL){
        V->AdjacencyList = E;
    }else{
        Edge* AdjacencyList = V->AdjacencyList;

        while(AdjacencyList->Next != NULL)
            AdjacencyList = AdjacencyList->Next;

        AdjacencyList->Next = E;
    }
}

void PrintGraph(Graph* G){
    Vertex* V = NULL;
    Edge*   E = NULL;

    if((V = G->Vertices) == NULL)
        return;

    while(V != NULL){
        printf("%c : ",V->Data);

        if((E = V->AdjacencyList) == NULL){
            V = V->Next;
            printf("\n");
            continue;
        }

        while(E != NULL){
            printf("%c[%d] ",E->Target->Data,E->Weight);
            E = E->Next;
        }

        printf("\n");

        V = V->Next;
    }
    printf("\n");
}

void DFS(Vertex* V){
    Edge* E = NULL;

    printf("%d ",V->Data);

    V->Visited = Visited;

    E = V->AdjacencyList;

    while(E != NULL){
        if(E->Target != NULL && E->Target->Visited == NotVisited)
            DFS(E->Target);
        
        E = E->Next;
    }
}//깊이 우선 탐색(Depth First Search) stack이용

int main(void){
    Graph* G = CreateGraph();

    Vertex* V1 = CreateVertex('1');
    Vertex* V2 = CreateVertex('2');
    Vertex* V3 = CreateVertex('3');
    Vertex* V4 = CreateVertex('4');
    Vertex* V5 = CreateVertex('5');

    AddVertex(G, V1);
    AddVertex(G, V2);
    AddVertex(G, V3);
    AddVertex(G, V4);
    AddVertex(G, V5);

    AddEdge(V1,CreateEdge(V1, V2, 0));
    AddEdge(V1,CreateEdge(V1, V3, 0));
    AddEdge(V1,CreateEdge(V1, V4, 0));
    AddEdge(V1,CreateEdge(V1, V5, 0));
    
    AddEdge(V2,CreateEdge(V2, V1, 0));
    AddEdge(V2,CreateEdge(V2, V3, 0));
    AddEdge(V2,CreateEdge(V2, V5, 0));

    AddEdge(V3,CreateEdge(V3, V1, 0));
    AddEdge(V3,CreateEdge(V3, V2, 0));
    
    AddEdge(V4,CreateEdge(V4, V1, 0));
    AddEdge(V4,CreateEdge(V4, V5, 0));

    AddEdge(V5,CreateEdge(V5, V1, 0));
    AddEdge(V5,CreateEdge(V5, V2, 0));
    AddEdge(V5,CreateEdge(V5, V4, 0));

    PrintGraph(G);

    DestroyGraph(G);

    return 0;
}