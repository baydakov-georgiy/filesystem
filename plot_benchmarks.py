import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

plt.style.use('seaborn-v0_8-darkgrid')
plt.rcParams['figure.figsize'] = (12, 6)
plt.rcParams['font.size'] = 10

def plot_operation(csv_file, title, ylabel):
    df = pd.read_csv(csv_file)
    
    plt.figure()
    plt.plot(df['size'], df['best'], marker='o', linewidth=2, label='Best Case', color='green')
    plt.plot(df['size'], df['average'], marker='s', linewidth=2, label='Average Case', color='blue')
    plt.plot(df['size'], df['worst'], marker='^', linewidth=2, label='Worst Case', color='red')
    
    plt.xlabel('Number of Elements', fontsize=12, fontweight='bold')
    plt.ylabel(ylabel, fontsize=12, fontweight='bold')
    plt.title(title, fontsize=14, fontweight='bold')
    plt.legend(loc='best', fontsize=11)
    plt.grid(True, alpha=0.3)
    plt.tight_layout()
    
    output_file = csv_file.replace('.csv', '.png')
    plt.savefig(output_file, dpi=300, bbox_inches='tight')
    print(f"Saved: {output_file}")
    plt.close()

def plot_all_operations():
    plot_operation('benchmark_insert.csv', 
                   'AVL H-Tree: Insert Operation Performance',
                   'Time per Operation (nanoseconds)')
    
    plot_operation('benchmark_find.csv',
                   'AVL H-Tree: Find Operation Performance', 
                   'Time per Operation (nanoseconds)')
    
    plot_operation('benchmark_remove.csv',
                   'AVL H-Tree: Remove Operation Performance',
                   'Time per Operation (nanoseconds)')

def plot_combined():
    fig, axes = plt.subplots(1, 3, figsize=(18, 5))
    
    operations = [
        ('benchmark_insert.csv', 'Insert Operation', axes[0]),
        ('benchmark_find.csv', 'Find Operation', axes[1]),
        ('benchmark_remove.csv', 'Remove Operation', axes[2])
    ]
    
    for csv_file, title, ax in operations:
        df = pd.read_csv(csv_file)
        
        ax.plot(df['size'], df['best'], marker='o', linewidth=2, label='Best', color='green')
        ax.plot(df['size'], df['average'], marker='s', linewidth=2, label='Average', color='blue')
        ax.plot(df['size'], df['worst'], marker='^', linewidth=2, label='Worst', color='red')
        
        ax.set_xlabel('Number of Elements', fontsize=11, fontweight='bold')
        ax.set_ylabel('Time (ns)', fontsize=11, fontweight='bold')
        ax.set_title(title, fontsize=12, fontweight='bold')
        ax.legend(loc='best', fontsize=10)
        ax.grid(True, alpha=0.3)
    
    plt.suptitle('AVL H-Tree Performance Comparison', fontsize=14, fontweight='bold', y=1.02)
    plt.tight_layout()
    plt.savefig('benchmark_combined.png', dpi=300, bbox_inches='tight')
    print("Saved: benchmark_combined.png")
    plt.close()

def generate_statistics():
    with open('benchmark_statistics.txt', 'w') as f:
        f.write("=== AVL H-Tree Performance Statistics ===\n\n")
        
        for operation in ['insert', 'find', 'remove']:
            csv_file = f'benchmark_{operation}.csv'
            df = pd.read_csv(csv_file)
            
            f.write(f"{operation.upper()} Operation:\n")
            f.write(f"  Size range: {df['size'].min()} - {df['size'].max()} elements\n")
            f.write(f"  Best case:    {df['best'].min():.2f} - {df['best'].max():.2f} ns\n")
            f.write(f"  Average case: {df['average'].min():.2f} - {df['average'].max():.2f} ns\n")
            f.write(f"  Worst case:   {df['worst'].min():.2f} - {df['worst'].max():.2f} ns\n")
            f.write(f"  Worst/Best ratio at max size: {df['worst'].iloc[-1] / df['best'].iloc[-1]:.2f}x\n")
            f.write("\n")
    
    print("Saved: benchmark_statistics.txt")

if __name__ == '__main__':
    print("Generating performance graphs...")
    plot_all_operations()
    plot_combined()
    generate_statistics()
    print("\nAll graphs generated successfully!")
