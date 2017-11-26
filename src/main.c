#include "logpipe_in.h"

char	__LOGPIPE_VERSION_0_1_0[] = "0.1.0" ;
char	*__LOGPIPE_VERSION = __LOGPIPE_VERSION_0_1_0 ;

static void version()
{
	printf( "logpipe v%s build %s %s\n" , __LOGPIPE_VERSION , __DATE__ , __TIME__ );
	return;
}

static void usage()
{
	printf( "USAGE : logpipe -v\n" );
	printf( "        logpipe --role C --monitor-path (dir_path) [ --trace-file-space-size (max_file_count) ]\n" );
	printf( "                        --log-file (log_file) --log-level (DEBUG|INFO|WARN|ERROR|FATAL)\n" );
	printf( "                        --no-daemon\n" );
	return;
}

static int ParseCommandParameter( struct LogPipeEnv *p_env , int argc , char *argv[] )
{
	int		c ;
	
	int		nret = 0 ;
	
	SetLogLevel( LOGLEVEL_WARN );
	
	for( c = 1 ; c < argc ; c++ )
	{
		if( STRCMP( argv[c] , == , "-v" ) )
		{
			version();
			exit(0);
		}
		else if( STRCMP( argv[c] , == , "--role" ) && c + 1 < argc )
		{
			if( STRCMP(argv[c+1],==,"C") )
			{
				p_env->role = LOGPIPE_ROLE_COLLECTOR ;
			}
			else if( STRCMP(argv[c+1],==,"P") )
			{
				p_env->role = LOGPIPE_ROLE_PIPER ;
			}
			else if( STRCMP(argv[c+1],==,"S") )
			{
				p_env->role = LOGPIPE_ROLE_DUMPSERVER ;
			}
			else
			{
				printf( "*** ERROR : parse invalid value '%s' of command parameter '--role'\n" , argv[c+1] );
				return -1;
			}
			c++;
		}
		else if( STRCMP( argv[c] , == , "--monitor-path" ) && c + 1 < argc )
		{
			strncpy( p_env->role_context.collector.monitor_path , argv[c+1] , sizeof(p_env->role_context.collector.monitor_path)-1 );
			c++;
		}
		else if( STRCMP( argv[c] , == , "--trace-file-space-size" ) && c + 1 < argc )
		{
			p_env->role_context.collector.trace_file_space_size = atoi(argv[c+1]) ;
			c++;
		}
		else if( STRCMP( argv[c] , == , "--log-file" ) && c + 1 < argc )
		{
			strncpy( p_env->log_pathfilename , argv[c+1] , sizeof(p_env->log_pathfilename)-1 );
			c++;
		}
		else if( STRCMP( argv[c] , == , "--log-level" ) && c + 1 < argc )
		{
			if( STRCMP(argv[c+1],==,"DEBUG") )
			{
				SetLogLevel( LOGLEVEL_DEBUG );
			}
			else if( STRCMP(argv[c+1],==,"INFO") )
			{
				SetLogLevel( LOGLEVEL_INFO );
			}
			else if( STRCMP(argv[c+1],==,"WARN") )
			{
				SetLogLevel( LOGLEVEL_WARN );
			}
			else if( STRCMP(argv[c+1],==,"ERROR") )
			{
				SetLogLevel( LOGLEVEL_ERROR );
			}
			else if( STRCMP(argv[c+1],==,"FATAL") )
			{
				SetLogLevel( LOGLEVEL_FATAL );
			}
			else
			{
				printf( "*** ERROR : parse invalid value '%s' of command parameter '--log-level'\n" , argv[c+1] );
				return -1;
			}
			c++;
		}
		else if( STRCMP( argv[c] , == , "--no-daemon" ) )
		{
			p_env->no_daemon = 1 ;
		}
		else
		{
			printf( "*** ERROR : invalid command parameter '%s'\n" , argv[c] );
			usage();
			exit(1);
		}
	}
	
	if( p_env->role == LOGPIPE_ROLE_COLLECTOR )
	{
		if( p_env->role_context.collector.monitor_path[0] == '\0' )
		{
			printf( "*** ERROR : need parameter '--monitor-path' for --role C\n" );
			return -1;
		}
		
		nret = access( p_env->role_context.collector.monitor_path , R_OK ) ;
		if( nret == -1 )
		{
			printf( "*** ERROR : path '%s' invalid\n" , p_env->role_context.collector.monitor_path );
			return -1;
		}
		
		if( p_env->role_context.collector.trace_file_space_size <= 0 )
			p_env->role_context.collector.trace_file_space_size = LOGPIPE_FILE_TRACE_SPACE_DEFAULT_SIZE ;
		p_env->role_context.collector.trace_file_space_total_size = p_env->role_context.collector.trace_file_space_size * LOGPIPE_FILE_TRACE_SPACE_EXPANSION_FACTOR ;
		
		printf( "role : LOGPIPE_ROLE_COLLECTOR\n" );
		printf( "monitor_path : %s\n" , p_env->role_context.collector.monitor_path );
		printf( "trace_file_space_size : %d\n" , p_env->role_context.collector.trace_file_space_size );
	}
	else
	{
		printf( "*** ERROR : role '%c' invalid\n" , p_env->role );
		return -1;
	}
	
	return 0;
}

int main( int argc , char *argv[] )
{
	struct LogPipeEnv	env , *p_env = & env ;
	
	int			nret = 0 ;
	
	setbuf( stdout , NULL );
	
	memset( p_env , 0x00 , sizeof(struct LogPipeEnv) );
	
	nret = ParseCommandParameter( p_env , argc , argv ) ;
	if( nret )
		return -nret;
	
	nret = InitEnvironment( p_env ) ;
	if( nret )
		return -nret;
	
	printf( "set log file to '%s'\n" , p_env->log_pathfilename );
	SetLogFile( "%s" , p_env->log_pathfilename );
	
	if( p_env->no_daemon )
	{
		nret = monitor( p_env ) ;
	}
	else
	{
		nret = BindDaemonServer( & _monitor , (void*)p_env , 1 ) ;
	}
	
	CleanEnvironment( p_env );
	
	return -nret;
}
