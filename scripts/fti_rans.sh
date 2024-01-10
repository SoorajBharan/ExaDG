find ./ -type f -exec sed -i 's/FTI/IncRANS/g' {} +
find ./ -type f -exec sed -i 's/incompressible_flow_with_transport/incompressible_rans/g' {} +
find ./ -type f -exec sed -i 's/INCOMPRESSIBLE_FLOW_WITH_TRANSPORT/INCOMPRESSIBLE_RANS/g' {} +
