import type { Meta, StoryObj } from '@storybook/react';
import React from 'react';

import { NetworkStatsDisplay } from './NetworkStatsDisplay';

const meta: Meta<typeof NetworkStatsDisplay> = {
  title: 'Components/NetworkStatsDisplay',
  component: NetworkStatsDisplay,
  parameters: {
    layout: 'centered',
    docs: {
      description: {
        component: 'A network statistics display component that shows real-time network metrics including connection statistics, bandwidth usage, and connection status.',
      },
    },
  },
  tags: ['autodocs'],
  argTypes: {
    autoRefresh: {
      control: 'boolean',
      description: 'Whether to automatically refresh network stats',
    },
    refreshInterval: {
      control: { type: 'number', min: 1000, max: 60000, step: 1000 },
      description: 'Interval in milliseconds for refreshing data',
    },
    showConnectionStats: {
      control: 'boolean',
      description: 'Whether to show connection statistics',
    },
    showBandwidthUsage: {
      control: 'boolean',
      description: 'Whether to show bandwidth usage',
    },
    showConnectionStatus: {
      control: 'boolean',
      description: 'Whether to show connection status',
    },
    className: {
      control: 'text',
      description: 'Additional CSS class name',
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
    autoRefresh: true,
    refreshInterval: 3000,
    showConnectionStats: true,
    showBandwidthUsage: true,
    showConnectionStatus: true,
  },
};

// Connection stats only
export const ConnectionStatsOnly: Story = {
  args: {
    autoRefresh: true,
    refreshInterval: 2000,
    showConnectionStats: true,
    showBandwidthUsage: false,
    showConnectionStatus: false,
  },
  parameters: {
    docs: {
      description: {
        story: 'Network display showing only connection statistics.',
      },
    },
  },
};

// Bandwidth focused
export const BandwidthFocused: Story = {
  args: {
    autoRefresh: true,
    refreshInterval: 1000,
    showConnectionStats: false,
    showBandwidthUsage: true,
    showConnectionStatus: false,
  },
  parameters: {
    docs: {
      description: {
        story: 'Network display focused on bandwidth usage with fast refresh rate.',
      },
    },
  },
};

// Status only
export const StatusOnly: Story = {
  args: {
    autoRefresh: true,
    refreshInterval: 5000,
    showConnectionStats: false,
    showBandwidthUsage: false,
    showConnectionStatus: true,
  },
  parameters: {
    docs: {
      description: {
        story: 'Network display showing only connection status.',
      },
    },
  },
};

// Manual refresh
export const ManualRefresh: Story = {
  args: {
    autoRefresh: false,
    refreshInterval: 3000,
    showConnectionStats: true,
    showBandwidthUsage: true,
    showConnectionStatus: true,
  },
  parameters: {
    docs: {
      description: {
        story: 'Network display with manual refresh - data must be refreshed manually.',
      },
    },
  },
};

// Custom styled
export const CustomStyled: Story = {
  args: {
    autoRefresh: true,
    refreshInterval: 3000,
    showConnectionStats: true,
    showBandwidthUsage: true,
    showConnectionStatus: true,
    className: 'custom-network-stats',
    style: {
      border: '2px solid #28a745',
      borderRadius: '12px',
      padding: '24px',
      backgroundColor: '#f8fff9',
      boxShadow: '0 6px 12px rgba(40, 167, 69, 0.15)',
      minWidth: '400px',
    },
  },
  parameters: {
    docs: {
      description: {
        story: 'Network stats display with custom styling applied.',
      },
    },
  },
};

// With error handling
export const WithErrorHandling: Story = {
  args: {
    autoRefresh: true,
    refreshInterval: 3000,
    showConnectionStats: true,
    showBandwidthUsage: true,
    showConnectionStatus: true,
    onError: (error: Error) => {
      console.error('Network Stats Error:', error);
      // In a real application, you might show an error message or send to error tracking
    },
  },
  parameters: {
    docs: {
      description: {
        story: 'Network stats display with error handling callback.',
      },
    },
  },
};

// Fast refresh
export const FastRefresh: Story = {
  args: {
    autoRefresh: true,
    refreshInterval: 500,
    showConnectionStats: true,
    showBandwidthUsage: true,
    showConnectionStatus: true,
  },
  parameters: {
    docs: {
      description: {
        story: 'Network stats display with very fast refresh rate (500ms) for real-time monitoring.',
      },
    },
  },
};

// Slow refresh
export const SlowRefresh: Story = {
  args: {
    autoRefresh: true,
    refreshInterval: 15000,
    showConnectionStats: true,
    showBandwidthUsage: true,
    showConnectionStatus: true,
  },
  parameters: {
    docs: {
      description: {
        story: 'Network stats display with slow refresh rate (15 seconds) for less frequent updates.',
      },
    },
  },
};

// Minimal display
export const Minimal: Story = {
  args: {
    autoRefresh: true,
    refreshInterval: 5000,
    showConnectionStats: false,
    showBandwidthUsage: true,
    showConnectionStatus: true,
  },
  parameters: {
    docs: {
      description: {
        story: 'Minimal network display showing only bandwidth and status.',
      },
    },
  },
};

// Comprehensive monitoring
export const ComprehensiveMonitoring: Story = {
  args: {
    autoRefresh: true,
    refreshInterval: 2000,
    showConnectionStats: true,
    showBandwidthUsage: true,
    showConnectionStatus: true,
    style: {
      maxWidth: '600px',
      padding: '20px',
      border: '1px solid #dee2e6',
      borderRadius: '8px',
      backgroundColor: '#ffffff',
    },
  },
  parameters: {
    docs: {
      description: {
        story: 'Comprehensive network monitoring with all metrics displayed.',
      },
    },
  },
};