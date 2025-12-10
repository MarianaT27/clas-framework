#!/usr/bin/bash
#SBATCH --job-name=myjob
#SBATCH --partition=priority
#SBATCH --time=00:10:00
#SBATCH --output=/w/hallb-scshelf2102/clas12/ouillon/tmp/test-framework4/build/logs/pi0_LD2_OB_0_%j.out
#SBATCH --error=/w/hallb-scshelf2102/clas12/ouillon/tmp/test-framework4/build/logs/pi0_LD2_OB_0_%j.err
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=1
#SBATCH --mem=2G

echo "Host: $(hostname)"
echo "Start: $(date)"

# >>> Crucial line for bash in SLURM! <<<
source /etc/profile.d/modules.sh

module purge
module load gcc/12.2.0
module load python/3.10

echo "Finished: $(date)"
