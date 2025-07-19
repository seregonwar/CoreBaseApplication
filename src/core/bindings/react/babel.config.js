module.exports = {
  presets: [
    [
      '@babel/preset-env',
      {
        targets: {
          node: '16',
          browsers: [
            'last 2 versions',
            'not dead',
            'not < 2%',
            'not ie 11'
          ]
        },
        modules: false, // Let bundler handle modules
        useBuiltIns: 'usage',
        corejs: 3,
        shippedProposals: true
      }
    ],
    [
      '@babel/preset-react',
      {
        runtime: 'automatic', // Use new JSX transform
        development: process.env.NODE_ENV === 'development'
      }
    ],
    [
      '@babel/preset-typescript',
      {
        isTSX: true,
        allExtensions: true,
        allowNamespaces: true,
        allowDeclareFields: true
      }
    ]
  ],
  plugins: [
    '@babel/plugin-proposal-class-properties',
    '@babel/plugin-proposal-object-rest-spread',
    '@babel/plugin-proposal-optional-chaining',
    '@babel/plugin-proposal-nullish-coalescing-operator',
    '@babel/plugin-proposal-logical-assignment-operators',
    '@babel/plugin-syntax-dynamic-import',
    [
      '@babel/plugin-transform-runtime',
      {
        corejs: false,
        helpers: true,
        regenerator: true,
        useESModules: false
      }
    ]
  ],
  env: {
    test: {
      presets: [
        [
          '@babel/preset-env',
          {
            targets: {
              node: 'current'
            },
            modules: 'commonjs'
          }
        ],
        [
          '@babel/preset-react',
          {
            runtime: 'automatic'
          }
        ],
        '@babel/preset-typescript'
      ],
      plugins: [
        '@babel/plugin-proposal-class-properties',
        '@babel/plugin-proposal-object-rest-spread',
        '@babel/plugin-proposal-optional-chaining',
        '@babel/plugin-proposal-nullish-coalescing-operator',
        '@babel/plugin-transform-modules-commonjs'
      ]
    },
    development: {
      plugins: [
        'react-refresh/babel'
      ]
    },
    production: {
      plugins: [
        [
          'babel-plugin-transform-react-remove-prop-types',
          {
            mode: 'remove',
            removeImport: true,
            additionalLibraries: ['react-immutable-proptypes']
          }
        ],
        '@babel/plugin-transform-react-constant-elements',
        '@babel/plugin-transform-react-inline-elements'
      ]
    }
  },
  assumptions: {
    setPublicClassFields: true,
    privateFieldsAsProperties: true
  },
  sourceType: 'module',
  retainLines: false,
  compact: process.env.NODE_ENV === 'production'
};