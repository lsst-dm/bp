#!/bin/bash
eups declare --force -r ./base base bp
eups declare --force -r ./sconsUtils sconsUtils bp
eups declare --force -r ./sconsUtils scons bp
eups declare --force -r ./bputils bputils bp
eups declare --force -r ./pex/exceptions pex_exceptions bp
eups declare --force -r ./utils utils bp
eups declare --force -r ./daf/base daf_base bp
eups declare --force -r ./pex/logging pex_logging bp
