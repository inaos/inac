# Logging
## Overview
INAC provides a logging facility with a runtime and compile time configuration,
4 levels and supports multiple targets.

## Usage
Initialize the logging facility. Initialization can be invoked only
once per running process. Subsequents initialization don't have any
impact.

Initialize default configuration. Print levels INFO, WARNING, DEBUG to the 
standard output stream and ERROR level to the standard error output stream.
Initialization can only be invoked once par process.

    ina_log_init(NULL);

Initialize the logging using configuration string.

    ina_log_init(cfg_string);
   
Initialize the logging facility from a file.
  
    ina_log_init_from_file("log.conf");
  
Log to the default log context.

    INA_LOG_INFO("this is a simple info message");
    INA_LOG_WARNING("this is a warning");
    INA_LOG_ERROR("this is an error message");
    INA_LOG_DEBUG("this is a debug message");
    
### Custom log context
Logging using a custom context. Create a log context
    
    ina_log_ctx_t *ctx;
    
    ina_log_ctx_new("mylib", &ctx);
    
Log to the context using the INA_LOG_CTX_XXX macros.

    INA_LOG_CTX_DEBUG(ctx, "buffer size %d", bufsiz);

Destroy the context before your exit
    
    ina_log_destroy(&ctx);
    
### Configuration

The configuration consists of a global section and rule-based configuration.
The global section is required, settings are all optional. 
Currently, "buffer_size" is the only supported setting. 

    global {
        -- Buffer used to write, when full the content will be flushed to the disk
        buffer_size=4096
    }
 
A rule consists of a category name and logging level separated by a period.
A rule defines a target and option to apply.
Name and level can be defined with wildcard *.  
   
    -- all categories and levels to "all.log"
    rule "*.*" {
        target="ell.log"
    }
    
    -- all categories and ERROR level to stderr
    rule "*.ERROR" {
        target=">stderr"
    }    

    -- all categories and levels to stderr
    rule "*.*" {
        target=">stdout"
    }    

    -- category "mydriver" and level  ERROR to syslog with 
    -- identifier "mydriver"
    rule "mydriver.ERROR" {
        target=">syslog"
        syslog_ident="mydriver"
    }    

    -- category "test" and level DEBUG to "test_debug.log", buffer size 1024 
    -- bytes
    rule "test.DEBUG" {
        target="test_debug.log"
        buffer_size=1024
    }
    
    -- caterory "test" and all levels to "test_all.log", truncate fille
    rule "test.*" {
        target="test_all.log"
        truncate="true"
    }

### Compile time configuration
 * `INA_LOG_ENABLED`    : Enable/disable logging. Default enabled.
 * `INA_LOG_LEVEL`      : Set log level from 1 (errors) to 4(debug). 
                          Default 3 (info).
