The way this is organized:

Each folder is a "Module"

A module is a collection of components & systems, typically one file per component.
This file defines the component and ANY systems that EDIT the components in that specific file.
Other components can/should be READ but not written to.

Each defined component should exists within a namespaced defined by the enclosing module.

Root.h is a file the defines the global ECS and any macros needed to define/run systems. 

