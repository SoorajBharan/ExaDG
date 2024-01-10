find ./ -type f -exec sed -i 's/ConvDiff/RTE/g' {} +
find ./ -type f -exec sed -i 's/convection_diffusion/rans_transport/g' {} +
find ./ -type f -exec sed -i 's/CONVECTION_DIFFUSION/RANS_TRANSPORT/g' {} +
