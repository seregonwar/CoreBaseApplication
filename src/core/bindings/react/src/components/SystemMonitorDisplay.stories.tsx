import type { Meta, StoryObj } from '@storybook/react';
import React from 'react';

import { SystemMonitorDisplay } from './SystemMonitorDisplay';
import { SystemAlert } from '../types';

const meta: Meta<typeof SystemMonitorDisplay> = {
  title: 'Components/SystemMonitorDisplay',
  component: SystemMonitorDisplay,
  parameters: {
    layout: 'centered',
    docs: {
      description: {
        component: 'A comprehensive system monitoring display component that shows real-time system metrics including CPU, memory, disk, network, and other performance indicators.',
      },
    },
  },
  tags: ['autodocs'],
  argTypes: {
    autoStart: {
      control: 'boolean',
      description: 'Whether to automatically start monitoring when component mounts',
    },
    refreshInterval: {
      control: { type: 'number', min: 1000, max: 60000, step: 1000 },
      description: 'Interval in milliseconds for refreshing data',
    },
    showCpu: {
      control: 'boolean',
      description: 'Whether to show CPU metrics',
    },
    showMemory: {
      control: 'boolean',
      description: 'Whether to show memory metrics',
    },
    showDisk: {
      control: 'boolean',
      description: 'Whether to show disk metrics',
    },
    showNetwork: {
      control: 'boolean',
      description: 'Whether to show network metrics',
    },
    showProcess: {
      control: 'boolean',
      description: 'Whether to show process metrics',
    },
    showLoad: {
      control: 'boolean',
      description: 'Whether to show system load metrics',
    },
    showTemperature: {
      control: 'boolean',
      description: 'Whether to show temperature metrics',
    },
    showPerformance: {
      control: 'boolean',
      description: 'Whether to show performance metrics',
    },
    className: {
      control: 'text',
      description: 'Additional CSS class name',
    },
    onAlert: {
      action: 'alert',
      description: 'Callback function called when an alert is triggered',
    },
    onError: {
      action: 'error',
      description: 'Callback function called when an error occurs',
    },
  },
};

export default meta;
type Story = StoryObj<typeof meta>;

// Default story
export const Default: Story = {
  args: {
    autoStart: true,
    refreshInterval: 5000,
    showCpu: true,
    showMemory: true,
    showDisk: true,
    showNetwork: true,
    showProcess: true,
    showLoad: true,
    showTemperature: true,
    showPerformance: true,
  },
};

// Minimal display
export const Minimal: Story = {
  args: {
    autoStart: true,
    refreshInterval: 3000,
    showCpu: true,
    showMemory: true,
    showDisk: false,
    showNetwork: false,
    showProcess: false,
    showLoad: false,
    showTemperature: false,
    showPerformance: false,
  },
  parameters: {
    docs: {
      description: {
        story: 'A minimal system monitor showing only CPU and memory metrics.',
      },
    },
  },
};

// CPU and Memory only
export const CpuMemoryOnly: Story = {
  args: {
    autoStart: true,
    refreshInterval: 2000,
    showCpu: true,
    showMemory: true,
    showDisk: false,
    showNetwork: false,
    showProcess: false,
    showLoad: false,
    showTemperature: false,
    showPerformance: false,
  },
  parameters: {
    docs: {
      description: {
        story: 'System monitor focused on CPU and memory usage with fast refresh rate.',
      },
    },
  },
};

// Network focused
export const NetworkFocused: Story = {
  args: {
    autoStart: true,
    refreshInterval: 1000,
    showCpu: false,
    showMemory: false,
    showDisk: false,
    showNetwork: true,
    showProcess: false,
    showLoad: false,
    showTemperature: false,
    showPerformance: false,
  },
  parameters: {
    docs: {
      description: {
        story: 'System monitor focused on network metrics with very fast refresh rate.',
      },
    },
  },
};

// Performance monitoring
export const PerformanceMonitoring: Story = {
  args: {
    autoStart: true,
    refreshInterval: 5000,
    showCpu: true,
    showMemory: true,
    showDisk: true,
    showNetwork: false,
    showProcess: true,
    showLoad: true,
    showTemperature: true,
    showPerformance: true,
  },
  parameters: {
    docs: {
      description: {
        story: 'Comprehensive performance monitoring including temperature and load metrics.',
      },
    },
  },
};

// Custom styled
export const CustomStyled: Story = {
  args: {
    autoStart: true,
    refreshInterval: 5000,
    showCpu: true,
    showMemory: true,
    showDisk: true,
    showNetwork: true,
    showProcess: false,
    showLoad: false,
    showTemperature: false,
    showPerformance: false,
    className: 'custom-monitor',
    style: {
      border: '2px solid #007acc',
      borderRadius: '8px',
      padding: '20px',
      backgroundColor: '#f8f9fa',
      boxShadow: '0 4px 6px rgba(0, 0, 0, 0.1)',
    },
  },
  parameters: {
    docs: {
      description: {
        story: 'System monitor with custom styling applied.',
      },
    },
  },
};

// With alert handling
export const WithAlertHandling: Story = {
  args: {
    autoStart: true,
    refreshInterval: 3000,
    showCpu: true,
    showMemory: true,
    showDisk: true,
    showNetwork: true,
    showProcess: true,
    showLoad: true,
    showTemperature: true,
    showPerformance: true,
    onAlert: (alert: SystemAlert) => {
      console.log('System Alert:', alert);
      // In a real application, you might show a notification or send to monitoring service
    },
    onError: (error: Error) => {
      console.error('Monitor Error:', error);
      // In a real application, you might show an error message or send to error tracking
    },
  },
  parameters: {
    docs: {
      description: {
        story: 'System monitor with alert and error handling callbacks.',
      },
    },
  },
};

// Manual control
export const ManualControl: Story = {
  args: {
    autoStart: false,
    refreshInterval: 5000,
    showCpu: true,
    showMemory: true,
    showDisk: true,
    showNetwork: true,
    showProcess: true,
    showLoad: true,
    showTemperature: true,
    showPerformance: true,
  },
  parameters: {
    docs: {
      description: {
        story: 'System monitor with manual control - monitoring must be started manually.',
      },
    },
  },
};

// Fast refresh
export const FastRefresh: Story = {
  args: {
    autoStart: true,
    refreshInterval: 500,
    showCpu: true,
    showMemory: true,
    showDisk: false,
    showNetwork: true,
    showProcess: false,
    showLoad: false,
    showTemperature: false,
    showPerformance: false,
  },
  parameters: {
    docs: {
      description: {
        story: 'System monitor with very fast refresh rate (500ms) for real-time monitoring.',
      },
    },
  },
};

// Slow refresh
export const SlowRefresh: Story = {
  args: {
    autoStart: true,
    refreshInterval: 30000,
    showCpu: true,
    showMemory: true,
    showDisk: true,
    showNetwork: true,
    showProcess: true,
    showLoad: true,
    showTemperature: true,
    showPerformance: true,
  },
  parameters: {
    docs: {
      description: {
        story: 'System monitor with slow refresh rate (30 seconds) for less frequent updates.',
      },
    },
  },
};