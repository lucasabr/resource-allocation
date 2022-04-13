HOW TO RUN
first type make
then type ./banker <filename>

Assignment uses the following files:

*** Banker.c ***

Description: reads the provided file and performs FIFO and Banker scheduler of resources on the input
Dependencies: stdio.h, stdlib.h, string.h, stdbool.h, list.h, request.h, resource.h, process.h

*** Process.h ***
Description: Defines the representation for the process struct
Dependencies: None

*** Request.h ***
Description: Defines the representation for the request struct (used for instructions)
Dependencies: None

*** Resource.h ***
Description: Defines the representation for the resource struct
Dependencies: None

*** List.h ***
Description: Defines a list representation
Dependencies: request.h, process.h

*** List.c ***
Description: Operations that the list uses (like add,remove, etc)
Dependencies: stdlib.h, stdio.h, string.h, list.h, process.h

