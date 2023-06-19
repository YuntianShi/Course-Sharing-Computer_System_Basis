#include "cachelab.h"
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


typedef struct line  *Ptrline;
typedef struct cache *Ptrcache;

// 定义line
// dirty位用于做块替换时，是否将cache的内容写回主存中，对题目要求不造成任何影响，所以不维护dirty位
struct line
{
    char val;  // V位 只需要1bit即可，只有0和1两种状态，用最小字节的变量类型char
    long long
        Tag;  // Tag位
              // 题目给出地址类型为64位，所以极端情况下Tag会占到62位，故使用long
              // long类型
    int LRU;  // LRU位
              // 为支持LRU替换策略而添加的位，LRU位的长度取决于关联度，极端情况下全相联，cache容量等于主存容量，这时LRU
};


// 定义cache
struct cache
{
    int s;  // 组序号所占位宽
    int E;  // 组的相联度
    int b;  // 偏移量位宽
    int t;  // tag号所占位宽

    Ptrline lines;  // cache主体部分
};

Ptrcache CreateCache( int s, int E, int b );
void     LoadOrSaveCache( Ptrcache  C,
                          long long Tag,
                          int       Index,
                          int      *hit_count,
                          int      *miss_count,
                          int      *eviction_count );
void     ModifyCache( Ptrcache  C,
                      long long Tag,
                      int       Index,
                      int      *hit_count,
                      int      *miss_count,
                      int      *eviction_count );
void     Index2LineRange( int *line_index_first,
                          int *line_index_end,
                          int  E,
                          int  Index );
int      TagHitLineRange( Ptrcache  C,
                          int       line_index_first,
                          int       line_index_end,
                          long long Tag );
int      GetValIsZeroAndMaxLRULineIndex( Ptrcache C,
                                         int      line_index_first,
                                         int      line_index_end,
                                         int     *line_index_val_is_zero,
                                         int     *line_index_max_LRU );
void     LRUUpdateLineRange( Ptrcache C,
                             int      line_index_first,
                             int      line_index_end,
                             int      LRU );


Ptrcache CreateCache( int s, int E, int b )
{
    // 实体化cache
    Ptrcache Cache = ( Ptrcache )malloc( sizeof( struct cache ) );
    Cache->s       = s;
    Cache->E       = E;
    Cache->b       = b;
    Cache->t       = 64 - s - b;
    int line_num   = ( 1 << Cache->s ) * Cache->E;
    Cache->lines   = ( Ptrline )malloc( sizeof( struct line ) * line_num );
    for ( int i = 0; i < line_num; i++ )
    {
        Cache->lines[ i ].val = 0;
        Cache->lines[ i ].Tag = 0;
        Cache->lines[ i ].LRU = 0;
    }
    return Cache;
}

void LoadOrSaveCache( Ptrcache  C,
                      long long Tag,
                      int       Index,
                      int      *hit_count,
                      int      *miss_count,
                      int      *eviction_count )
{
    // 模拟Load过程。输入已经解析好的地址的Index、Tag和offset段，判断是否命中。
    int line_index_first = -1;
    int line_index_end   = -1;
    // 解析对应组内一共有多少line, [line_index_first, line_index_end)
    Index2LineRange( &line_index_first, &line_index_end, C->E, Index );
    // 根据Tag判断是否命中并完成LRU的修改，命中返回1。
    if ( TagHitLineRange( C, line_index_first, line_index_end, Tag ) == 1 )
        ( *hit_count )++;  // 如果命中了，命中数增大
    else
    {
        // 如果没命中，进行替换策略
        int line_index_val_is_zero = line_index_first,
            line_index_max_LRU     = line_index_first;
        ( *miss_count )++;
        // 不存在val=0的line，根据LRU进行替换
        if ( GetValIsZeroAndMaxLRULineIndex(
                 C, line_index_first, line_index_end, &line_index_val_is_zero,
                 &line_index_max_LRU ) == 0 )
        {
            C->lines[ line_index_max_LRU ].Tag = Tag;
            LRUUpdateLineRange( C, line_index_first, line_index_end,
                                C->lines[ line_index_max_LRU ].LRU );
            C->lines[ line_index_max_LRU ].LRU = 0;  // hit后LRU置零
            ( *eviction_count )++;
        }
        else
        {
            // 存在val = 0的line，根据val进行放入
            C->lines[ line_index_val_is_zero ].val = 1;
            C->lines[ line_index_val_is_zero ].Tag = Tag;
            LRUUpdateLineRange(
                C, line_index_first, line_index_end,
                ( C->E + 1 ) );  // 放入val=0的line，把所有LRU加一
            C->lines[ line_index_val_is_zero ].LRU = 0;
        }
    }
}

void ModifyCache( Ptrcache  C,
                  long long Tag,
                  int       Index,
                  int      *hit_count,
                  int      *miss_count,
                  int      *eviction_count )
{
    // modify 本质上是先做一次Load再做一次Save
    LoadOrSaveCache( C, Tag, Index, hit_count, miss_count, eviction_count );
    LoadOrSaveCache( C, Tag, Index, hit_count, miss_count, eviction_count );
}

void Index2LineRange( int *line_index_first,
                      int *line_index_end,
                      int  E,
                      int  Index )
{
    // 组的关联度为E,计算第Index组所对应line的范围
    *line_index_first = Index * E;
    *line_index_end   = *line_index_first + E;
}

int TagHitLineRange( Ptrcache  C,
                     int       line_index_first,
                     int       line_index_end,
                     long long Tag )
{
    // 对组内所有line比较Tag判断是否命中
    for ( int line = line_index_first; line < line_index_end; line++ )
    {
        // Tag命中
        if ( C->lines[ line ].val == 1 && C->lines[ line ].Tag == Tag )
        {
            // 修改组内其他LRU位
            LRUUpdateLineRange( C, line_index_first, line_index_end,
                                C->lines[ line ].LRU );
            C->lines[ line ].LRU = 0;  // hit后LRU置零
            return 1;
        }
    }
    return 0;
}

int GetValIsZeroAndMaxLRULineIndex( Ptrcache C,
                                    int      line_index_first,
                                    int      line_index_end,
                                    int     *line_index_val_is_zero,
                                    int     *line_index_max_LRU )
{
    // 遍历区间[line_index_first,
    // line_index_end)的所有val位，得到最后一个为0的line_index若没有则返回0，得到LRU最大的line，
    int flag = 0;  // 标志是根据val=0进行替换，还是根据LRU进行替换
    int max_LRU = 0;
    for ( int line = line_index_first; line < line_index_end; line++ )
    {
        // line_index_max_LRU的维护
        if ( C->lines[ line ].val == 1 && C->lines[ line ].LRU > max_LRU )
        {
            *line_index_max_LRU = line;
            max_LRU             = C->lines[ line ].LRU;
        }
        // line_index_val_is_zero的维护
        if ( C->lines[ line ].val == 0 )
        {
            *line_index_val_is_zero = line;
            flag                    = 1;
        }
    }
    return flag;
}

void LRUUpdateLineRange( Ptrcache C,
                         int      line_index_first,
                         int      line_index_end,
                         int      LRU )
{
    // 对LRU小于输入LRU的line进行更新
    for ( int line = line_index_first; line < line_index_end; line++ )
    {
        if ( C->lines[ line ].val == 1 &&
             C->lines[ line ].LRU < LRU )  // 找到组内LRU值小于该line的line
        {
            if ( C->lines[ line ].LRU < C->E )  // 维护LRU的最大值
            {
                C->lines[ line ].LRU++;  // LRU增大
            }
            // 若LRU以达到最大则不动
        }
    }
}

void Printh( char *name )
{
    printf( "Usage: %s [-hv] -s <num> -E <num> -b <num> -t <file>\n", name );
    printf( "Options:\n" );
    printf( "  -h         Print this help message.\n" );
    printf( "  -v         Optional verbose flag.\n" );
    printf( "  -s <num>   Number of set index bits.\n" );
    printf( "  -E <num>   Number of lines per set.\n" );
    printf( "  -b <num>   Number of block offset bits.\n" );
    printf( "  -t <file>  Trace file.\n\n" );
    printf( "Examples:\n" );
    printf( "  linux>  ./csim-ref -s 4 -E 1 -b 4 -t traces/yi.trace\n" );
    printf( "  linux>  ./csim-ref -v -s 8 -E 2 -b 4 -t traces/yi.trace\n" );
    exit( 0 );
}

int main( int argc, char *argv[] )
{
    int hit_count = 0, miss_count = 0, eviction_count = 0;  // 输出结果
    int s, E, b;                                            // cache的参数
    long long _Tag, _Index;
    int       o;   // 读入命令行参数
    FILE     *fp;  // 读入文件句柄
    // 处理输入
    while ( ( o = getopt( argc, argv, "hvs:E:b:t:" ) ) != -1 )
    {
        switch ( o )
        {
            case 'h': Printh( argv[ 0 ] ); break;
            case 'v': break;
            case 's': s = *optarg - '0'; break;
            case 'E': E = *optarg - '0'; break;
            case 'b': b = *optarg - '0'; break;
            case 't':
                if ( ( fp = fopen( optarg, "r" ) ) == NULL )
                {
                    printf( "cannot open this file\n" );
                    exit( 0 );
                }
                break;
            default:
                printf( "Invalid option '%c'\n", o );
                Printh( argv[ 0 ] );
                break;
        }
    }
    Ptrcache  C = CreateCache( s, E, b );
    char      op;
    long long addr;
    int       size;
    char     *buf = ( char     *)malloc( 50 * sizeof( char ) );
    while ( fgets( buf, 100, fp ) != NULL )
    {
        if ( buf[ 0 ] == ' ' )
        {
            sscanf( buf, " %c %llx,%d", &op, &addr, &size );
            // 提取Tag区域和Index区域
            _Tag   = ( ( addr >> ( ( C->b ) + ( C->s ) ) ) & ~( -1 << C->t ) );
            _Index = ( ( addr >> ( C->b ) ) & ( ( 1 << ( C->s ) ) - 1 ) );
            switch ( op )
            {
                case 'I': break;
                case 'L':
                case 'S':
                    LoadOrSaveCache( C, _Tag, _Index, &hit_count, &miss_count,
                                     &eviction_count );
                    break;
                case 'M':
                    ModifyCache( C, _Tag, _Index, &hit_count, &miss_count,
                                 &eviction_count );
                    break;
            }
        }
    }
    printSummary( hit_count, miss_count, eviction_count );
    return 0;
}
